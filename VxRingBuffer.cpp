/**
 * @file    VxRingBuffer.cpp
 * <pre>
 * Copyright (c) 2018, Gaaagaa All rights reserved.
 * 
 * 文件名称：VxRingBuffer.cpp
 * 创建日期：2018年11月22日
 * 文件标识：
 * 文件摘要：环形缓存的实现。
 * 
 * 当前版本：1.0.0.0
 * 作    者：
 * 完成日期：2018年11月22日
 * 版本摘要：
 * 
 * 取代版本：
 * 原作者  ：
 * 完成日期：
 * 版本摘要：
 * </pre>
 */

#include "VxRingBuffer.h"
#include <stdlib.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////

#ifndef XASSERT
#ifdef DEBUG
#include <cassert>
#define XASSERT(xptr)    assert((xptr))
#else // !DEBUG
#define XASSERT(xptr)
#endif // DEBUG
#endif // XASSERT

////////////////////////////////////////////////////////////////////////////////

/**********************************************************/
/**
 * @brief 创建 环形缓存对象（使用 rbuf_destroy() 接口进行对象销毁）。
 *
 * @param [in ] xut_size    : 环形缓存 的缓存大小。
 * @param [in ] xfunc_mptr  : 内存申请操作的回调函数接口（若为 X_NULL，则使用标准 C 的 malloc() 接口进行分配内存）。
 * @param [in ] xht_context : 回调的上下文描述句柄。
 *
 * @return x_ringbuf_t *
 *         - 操作成功，返回 环形缓存 对象；
 *         - 操作失败，返回  X_NULL。
 */
x_ringbuf_t * rbuf_create(x_uint32_t xut_size, xfunc_ringbuf_malloc xfunc_mptr, x_handle_t xht_context)
{
    x_ringbuf_t * xrbt_sptr = X_NULL;
    if (xut_size <= 0)
    {
        return X_NULL;
    }

    if (X_NULL == xfunc_mptr)
    {
        xrbt_sptr = (x_ringbuf_t *)malloc(sizeof(x_ringbuf_t) + xut_size);
    }
    else
    {
        xrbt_sptr = (x_ringbuf_t *)xfunc_mptr(xht_context, sizeof(x_ringbuf_t) + xut_size);
    }

    if (X_NULL != xrbt_sptr)
    {
        xrbt_sptr->xut_vpos = 0;
        xrbt_sptr->xut_vlen = 0;
        xrbt_sptr->xut_size = xut_size;
        xrbt_sptr->xct_dptr = ((x_uchar_t *)xrbt_sptr) + sizeof(x_ringbuf_t);
    }

    return xrbt_sptr;
}

/**********************************************************/
/**
 * @brief 销毁 环形缓存对象（由 rbuf_create() 接口创建的对象）。
 *
 * @param [in ] xrbt_sptr   : 环形缓存对象。
 * @param [in ] xfunc_fptr  : 内存释放操作的回调函数接口（若为 X_NULL，则使用标准 C 的 free() 接口进行分配内存）。
 * @param [in ] xht_context : 回调的上下文描述句柄。
 *
 */
x_void_t rbuf_destroy(x_ringbuf_t * xrbt_sptr, xfunc_ringbuf_free xfunc_fptr, x_handle_t xht_context)
{
    if (X_NULL != xrbt_sptr)
    {
        if (X_NULL == xfunc_fptr)
        {
            free(xrbt_sptr);
        }
        else
        {
            xfunc_fptr(xht_context, (x_void_t *)xrbt_sptr);
        }
    }
}

/**********************************************************/
/**
 * @brief 读取环形缓存对象中的有效数据至指定的缓存中；
 *        该操作后，会修改环形缓存中的有效数据相关字段。
 *
 * @param [in ] xrbt_sptr : 环形缓存对象。
 * @param [out] xct_rptr  : 指定的数据接收缓存。
 * @param [in ] xut_size  : 数据接收缓存大小。
 *
 * @return x_uint32_t
 *         - 返回读取到的有效数据量（字节数）。
 */
x_uint32_t rbuf_read(x_ringbuf_t * xrbt_sptr, x_uchar_t * xct_rptr, x_uint32_t xut_size)
{
    x_uint32_t xut_rbytes = 0;
    x_uint32_t xut_cbytes = 0;

    XASSERT(RBUF_IS_VALID(xrbt_sptr));
    XASSERT((0 == xut_size) || ((xut_size > 0) && (X_NULL != xct_dptr)));

    if ((xut_size <= 0) || (xrbt_sptr->xut_vlen <= 0))
    {
        return 0;
    }

    xut_rbytes = (xut_size >= xrbt_sptr->xut_vlen) ? xrbt_sptr->xut_vlen : xut_size;

    if ((xrbt_sptr->xut_vpos + xut_rbytes) <= xrbt_sptr->xut_size)
    {
        memcpy(xct_rptr, xrbt_sptr->xct_dptr + xrbt_sptr->xut_vpos, xut_rbytes);
    }
    else
    {
        xut_cbytes = xrbt_sptr->xut_size - xrbt_sptr->xut_vpos;
        memcpy(xct_rptr, xrbt_sptr->xct_dptr + xrbt_sptr->xut_vpos, xut_cbytes);
        memcpy(xct_rptr + xut_cbytes, xrbt_sptr->xct_dptr, xut_rbytes - xut_cbytes);
    }

    xrbt_sptr->xut_vlen -= xut_rbytes;
    if (0 == xrbt_sptr->xut_vlen)
        xrbt_sptr->xut_vpos = 0;
    else
        xrbt_sptr->xut_vpos = (xrbt_sptr->xut_vpos + xut_rbytes) % xrbt_sptr->xut_size;

    return xut_rbytes;
}

/**********************************************************/
/**
 * @brief 向环形缓存对象写入数据；
 *        环形缓存剩余的空间量必须足够待写入的数据量，否则直接返回 0，忽略后续操作；
 *        该操作后，会修改环形缓存中的有效数据相关字段。
 *
 * @param [out] xrbt_sptr : 环形缓存对象。
 * @param [in ] xct_wptr  : 待写入数据的缓存。
 * @param [in ] xut_size  : 待写入数据的缓存大小。
 *
 * @return x_uint32_t
 *         - 返回写入的有效数据量（字节数）。
 */
x_uint32_t rbuf_write(x_ringbuf_t * xrbt_sptr, const x_uchar_t * xct_wptr, x_uint32_t xut_size)
{
    x_uint32_t xut_wpos = 0;
    x_uint32_t xut_wlen = 0;

    XASSERT(RBUF_IS_VALID(xrbt_sptr));
    XASSERT((0 == xut_size) || ((xut_size > 0) && (X_NULL != xct_wptr)));

    if ((xut_size <= 0) || (xut_size > (xrbt_sptr->xut_size - xrbt_sptr->xut_vlen)))
    {
        return 0;
    }

    xut_wpos = (xrbt_sptr->xut_vpos + xrbt_sptr->xut_vlen) % xrbt_sptr->xut_size;

    if ((xut_wpos + xut_size) <= xrbt_sptr->xut_size)
    {
        memcpy(xrbt_sptr->xct_dptr + xut_wpos, xct_wptr, xut_size);
    }
    else
    {
        xut_wlen = xrbt_sptr->xut_size - xut_wpos;
        memcpy(xrbt_sptr->xct_dptr + xut_wpos, xct_wptr, xut_wlen);
        memcpy(xrbt_sptr->xct_dptr, xct_wptr + xut_wlen, xut_size - xut_wlen);
    }

    xrbt_sptr->xut_vlen += xut_size;

    return xut_size;
}

/**********************************************************/
/**
 * @brief 抹除环形缓存中的头部数据。
 *
 * @param [out] xrbt_sptr  : 环形缓存对象。
 * @param [in ] xut_count  : 请求抹除的字节数。
 *
 * @return x_uint32_t
 *         - 返回抹除的字节数。
 */
x_uint32_t rbuf_erase_head(x_ringbuf_t * xrbt_sptr, x_uint32_t xut_count)
{
    x_uint32_t xut_bytes = 0;

    XASSERT(RBUF_IS_VALID(xrbt_sptr));

    if (xut_count <= 0)
    {
        return 0;
    }

    if (xut_count >= xrbt_sptr->xut_vlen)
    {
        xut_bytes = xrbt_sptr->xut_vlen;

        xrbt_sptr->xut_vpos = 0;
        xrbt_sptr->xut_vlen = 0;
    }
    else
    {
        xut_bytes = xut_count;

        xrbt_sptr->xut_vpos = (xrbt_sptr->xut_vpos + xut_count) % xrbt_sptr->xut_size;
        xrbt_sptr->xut_vlen -= xut_count;
    }

    return xut_bytes;
}

/**********************************************************/
/**
 * @brief 抹除环形缓存中的尾部数据。
 *
 * @param [out] xrbt_sptr  : 环形缓存对象。
 * @param [in ] xut_count  : 请求抹除的字节数。
 *
 * @return x_uint32_t
 *         - 返回抹除的字节数。
 */
x_uint32_t rbuf_erase_tail(x_ringbuf_t * xrbt_sptr, x_uint32_t xut_count)
{
    x_uint32_t xut_bytes = 0;

    XASSERT(RBUF_IS_VALID(xrbt_sptr));

    if (xut_count <= 0)
    {
        return 0;
    }

    if (xut_count >= xrbt_sptr->xut_vlen)
    {
        xut_bytes = xrbt_sptr->xut_vlen;

        xrbt_sptr->xut_vpos = 0;
        xrbt_sptr->xut_vlen = 0;
    }
    else
    {
        xut_bytes = xut_count;

        xrbt_sptr->xut_vlen -= xut_count;
    }

    return xut_bytes;
}
