/*******************************************************************
*  Copyright(c) 2019
*  All rights reserved.
*
*  文件名称:    gsws.h
*  简要描述:    帧转换
*
*  作者:  gongluck
*  说明:
*
*******************************************************************/

#ifndef __GSWS_H__
#define __GSWS_H__

#include "gavbase.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include <libswscale/swscale.h>

#ifdef __cplusplus
}
#endif

namespace gff
{
    class gsws : public gavbase
    {
    public:
        ~gsws();

        /*
         * @brief   清理资源
         * @return  错误码
        */
        int cleanup() override;

        /*
         * @brief               设置转换参数
         * @return              错误码
         * @param spixfmt[in]   输入格式
         * @param sw[in]        输入宽度
         * @param sh[in]        输入高度
         * @param dpixfmt[in]   输出格式
         * @param dw[in]        输出宽度
         * @param dh[in]        输出高度
        */
        int create_sws(AVPixelFormat spixfmt, int sw, int sh, AVPixelFormat dpixfmt, int dw, int dh);

        /*
         * @brief                   转换
         * @return                  成功返回转换行数，否则返回错误码
         * @param srcSlice[in]      输入数据
         * @param srcStride[in]     输入数据行大小
         * @param srcSliceY[in]     开始行
         * @param srcSliceH[in]     行高
         * @param dst[out]          输出数据
         * @param dstStride[out]    输出数据行大小
        */
        int scale(const uint8_t* const srcSlice[], const int srcStride[], int srcSliceY, int srcSliceH, uint8_t* const dst[], const int dstStride[]);

    private:
        SwsContext* swsctx_ = nullptr;
    };
}//gff

#endif//__GSWS_H__