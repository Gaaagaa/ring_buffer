/**
 * @file    VxRingBuffer.h
 * <pre>
 * Copyright (c) 2018, Gaaagaa All rights reserved.
 * 
 * 文件名称：VxRingBuffer.h
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

#ifndef __VXRINGBUFFER_H__
#define __VXRINGBUFFER_H__

#include "VxDType.h"

////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

////////////////////////////////////////////////////////////////////////////////

/**
 * @struct x_ringbuf_t
 * @brief  环形缓存的结构体描述信息。
 */
typedef struct x_ringbuf_t
{
    x_uint32_t   xut_vpos;  ///< 缓存中有效数据的起始位置
    x_uint32_t   xut_vlen;  ///< 缓存中有效数据的长度
    x_uint32_t   xut_size;  ///< 缓存大小
    x_uchar_t  * xct_dptr;  ///< 缓存地址
} x_ringbuf_t;

/** 判断环形缓存是否有效 */
#define RBUF_IS_VALID(xrbt_sptr)   ((X_NULL != (xrbt_sptr)) && \
                                    (X_NULL != (xrbt_sptr)->xct_dptr) && ((xrbt_sptr)->xut_size > 0) && \
                                    ((xrbt_sptr)->xut_vpos >= 0) && ((xrbt_sptr)->xut_vpos < (xrbt_sptr)->xut_size) && \
                                    ((xrbt_sptr)->xut_vlen <= (xrbt_sptr)->xut_size))

/** 判断环形缓存是否为 空 */
#define RBUF_IS_EMPTY(xrbt_sptr)   (0 == (xrbt_sptr)->xut_vlen)

/** 判断环形缓存是否为 满 */
#define RBUF_IS_FULL(xrbt_sptr)    ((xrbt_sptr)->xut_size == (xrbt_sptr)->xut_vlen)

/**********************************************************/
/**
 * @brief 用于 环形缓存的内存申请操作 回调函数类型（内存要释放时，请调用 xfunc_ringbuf_free 指向的接口）。
 * 
 * @param [in ] xht_context : 回调的上下文描述句柄。
 * @param [in ] xut_size    : 提交申请的内存字节数。
 * 
 * @return x_uchar_t *
 *         - 操作成功，返回申请到的内存地址；
 *         - 操作失败，返回 X_NULL。
 */
typedef x_uchar_t * (* xfunc_ringbuf_malloc)(x_handle_t xht_context, x_uint32_t xut_size);

/**********************************************************/
/**
 * @brief 用于 环形缓存的内存释放操作 回调函数类型（所释放的内存，即为 xfunc_ringbuf_malloc 所申请的）。
 *
 * @param [in ] xht_context : 回调的上下文描述句柄。
 * @param [in ] xpvt_ptr    : 要释放的内存地址。
 * 
 */
typedef x_void_t (* xfunc_ringbuf_free)(x_handle_t xht_context, x_void_t * xpvt_ptr);

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
x_ringbuf_t * rbuf_create(x_uint32_t xut_size, xfunc_ringbuf_malloc xfunc_mptr, x_handle_t xht_context);

/**********************************************************/
/**
 * @brief 销毁 环形缓存对象（由 rbuf_create() 接口创建的对象）。
 * 
 * @param [in ] xrbt_sptr   : 环形缓存对象。
 * @param [in ] xfunc_fptr  : 内存释放操作的回调函数接口（若为 X_NULL，则使用标准 C 的 free() 接口进行释放内存）。
 * @param [in ] xht_context : 回调的上下文描述句柄。
 * 
 */
x_void_t rbuf_destroy(x_ringbuf_t * xrbt_sptr, xfunc_ringbuf_free xfunc_fptr, x_handle_t xht_context);

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
x_uint32_t rbuf_read(x_ringbuf_t * xrbt_sptr, x_uchar_t * xct_rptr, x_uint32_t xut_size);

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
x_uint32_t rbuf_write(x_ringbuf_t * xrbt_sptr, const x_uchar_t * xct_wptr, x_uint32_t xut_size);

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
x_uint32_t rbuf_erase_head(x_ringbuf_t * xrbt_sptr, x_uint32_t xut_count);

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
x_uint32_t rbuf_erase_tail(x_ringbuf_t * xrbt_sptr, x_uint32_t xut_count);

////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}; // extern "C"
#endif // __cplusplus

////////////////////////////////////////////////////////////////////////////////

#endif // __VXRINGBUFFER_H__
