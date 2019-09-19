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

        // 销毁资源
        int cleanup() override;

        // 设置参数
        int create_sws(AVPixelFormat spixfmt, int sw, int sh, AVPixelFormat dpixfmt, int dw, int dh);

        // 转换
        int scale(const uint8_t* const srcSlice[], const int srcStride[], int srcSliceY, int srcSliceH, uint8_t* const dst[], const int dstStride[]);

    private:
        SwsContext* swsctx_ = nullptr;
    };
}//gff

#endif//__GSWS_H__