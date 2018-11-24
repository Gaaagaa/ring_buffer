/**
 * @file    main.cpp
 * <pre>
 * Copyright (c) 2018, Gaaagaa All rights reserved.
 * 
 * 文件名称：main.cpp
 * 创建日期：2018年11月22日
 * 文件标识：
 * 文件摘要：测试环形缓存的程序。
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
#include <stdio.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////

//====================================================================

// 
// main : 测试程序的入口函数
// 

int main(int argc, char * argv[])
{
    x_uchar_t  xct_buffer[TEXT_LEN_512] = { 0 };
    x_uint32_t xut_buflen = 0;

    x_ringbuf_t * xrbt_sptr = rbuf_create(256, X_NULL, X_NULL);
    if (X_NULL == xrbt_sptr)
    {
        printf("(X_NULL == xrbt_sptr)");
        return -1;
    }

    //======================================

    for (int i = 0; i < 100; ++i)
    {
        rbuf_write(xrbt_sptr, (x_uchar_t *)"0123456789", (x_uint32_t)strlen("0123456789"));
    }

    printf("1. write: xrbt_sptr[vpos, vlen, size] == [%3d, %3d, %3d]\n", xrbt_sptr->xut_vpos, xrbt_sptr->xut_vlen, xrbt_sptr->xut_size);
    rbuf_erase_head(xrbt_sptr, xrbt_sptr->xut_size);
    printf("1. erase_head: xrbt_sptr[vpos, vlen, size] == [%3d, %3d, %3d]\n", xrbt_sptr->xut_vpos, xrbt_sptr->xut_vlen, xrbt_sptr->xut_size);

    //======================================

    for (int i = 0; i < 10; ++i)
    {
        rbuf_write(xrbt_sptr, (x_uchar_t *)"0123456789", (x_uint32_t)strlen("0123456789"));
        printf("2. write: xrbt_sptr[vpos, vlen, size] == [%3d, %3d, %3d]\n", xrbt_sptr->xut_vpos, xrbt_sptr->xut_vlen, xrbt_sptr->xut_size);
    }

    for (int i = 0; i < 5; ++i)
    {
        memset(xct_buffer, 0, TEXT_LEN_512);
        rbuf_read(xrbt_sptr, xct_buffer, (x_uint32_t)strlen("0123456789"));
        printf("2. read: xrbt_sptr[vpos, vlen, size] == [%3d, %3d, %3d] => buffer: %s\n", xrbt_sptr->xut_vpos, xrbt_sptr->xut_vlen, xrbt_sptr->xut_size, (x_char_t *)xct_buffer);
    }

    //======================================

    for (int i = 0; i < 100; ++i)
    {
        rbuf_write(xrbt_sptr, (x_uchar_t *)"0123456789", (x_uint32_t)strlen("0123456789"));
    }
    printf("3. write: xrbt_sptr[vpos, vlen, size] == [%3d, %3d, %3d]\n", xrbt_sptr->xut_vpos, xrbt_sptr->xut_vlen, xrbt_sptr->xut_size);
    if (!RBUF_IS_FULL(xrbt_sptr))
    {
        rbuf_write(xrbt_sptr, (x_uchar_t *)"0123456789", xrbt_sptr->xut_size - xrbt_sptr->xut_vlen);
    }
    printf("3. full: xrbt_sptr[vpos, vlen, size] == [%3d, %3d, %3d]\n", xrbt_sptr->xut_vpos, xrbt_sptr->xut_vlen, xrbt_sptr->xut_size);

    rbuf_erase_tail(xrbt_sptr, xrbt_sptr->xut_vlen / 2);
    printf("3. erase_tail: xrbt_sptr[vpos, vlen, size] == [%3d, %3d, %3d]\n", xrbt_sptr->xut_vpos, xrbt_sptr->xut_vlen, xrbt_sptr->xut_size);

    memset(xct_buffer, 0, TEXT_LEN_512);
    xut_buflen = rbuf_read(xrbt_sptr, xct_buffer, TEXT_LEN_512);
    printf("3. read: xrbt_sptr[vpos, vlen, size] == [%3d, %3d, %3d] => buffer[%d]: %s\n", xrbt_sptr->xut_vpos, xrbt_sptr->xut_vlen, xrbt_sptr->xut_size, xut_buflen, (x_char_t *)xct_buffer);

    rbuf_erase_tail(xrbt_sptr, xrbt_sptr->xut_size);
    printf("3. erase_tail: xrbt_sptr[vpos, vlen, size] == [%3d, %3d, %3d]\n", xrbt_sptr->xut_vpos, xrbt_sptr->xut_vlen, xrbt_sptr->xut_size);

    //======================================

    rbuf_destroy(xrbt_sptr, X_NULL, X_NULL);
    xrbt_sptr = X_NULL;

    return 0;
}
