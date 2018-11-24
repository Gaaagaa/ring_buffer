最近在做网络数据收发操作时，用到环形缓冲区的数据结构，网上看了很多人的实现方式，总觉得不妥，于是决定自己亲自写一个，适用于自己的项目。

## 结构定义

先看一张关于环形队列的描述图（如下），它是一种FIFO（先入先出）的队列，其明确队列的容量，存储结构上表现为头尾相连，使用起始位置和结束位置标识队列节点的存储区段。

![环形队列](https://i.imgur.com/aSTMRIq.jpg)

更多关于环形队列的描述，这里就不再重复，可至 [环形缓冲器(FIFO)](http://home.eeworld.com.cn/my/space-uid-346593-blogid-239256.html) 查看。

我给出相关代码的结构定义如下：
```
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
```
从上面的结构体定义中，可以看出，数据区段的**结束位置**，使用了**数据区段的长度**来替换了，这样做，可避免 **起始位置** 与 **结束位置** 重叠在一起时，表示不清 **环形缓存** 是 **满区段** 还是 **空区段**。

## 操作接口
```
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
```
对于 `rbuf_create()` 与 `rbuf_destroy()` 中加入了 **内存 申请/释放 的回调操作函数** 的参数，这样方便外部介入内存管理操作（使用内存池进行内存的分配与释放操作）。

## 实现细节
```
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
```

## 源码下载
ring_buffer: [https://github.com/Gaaagaa/ring_buffer](https://github.com/Gaaagaa/ring_buffer)

## 扩展阅读

无锁环形队列的一种高效实现：[https://www.cnblogs.com/dodng/p/4367791.html](https://www.cnblogs.com/dodng/p/4367791.html)

