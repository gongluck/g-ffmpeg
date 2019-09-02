/*******************************************************************
*  Copyright(c) 2019
*  All rights reserved.
*
*  文件名称:    gsws.cpp
*  简要描述:    帧转换
*
*  作者:  gongluck
*  说明:
*
*******************************************************************/

#include "gsws.h"
#include "gutil.h"

namespace gff
{
    gsws::~gsws()
    {
        cleanup();
    }

    int gsws::cleanup()
    {
        LOCK();

        sws_freeContext(swsctx_);
        swsctx_ = nullptr;

        return 0;
    }

    int gsws::create_sws(AVPixelFormat spixfmt, int sw, int sh, AVPixelFormat dpixfmt, int dw, int dh)
    {
        LOCK();

        cleanup();
        swsctx_ = sws_getContext(sw, sh, spixfmt, dw, dh, dpixfmt, SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
        if (swsctx_ == nullptr)
        {
            CHECKFFRET(AVERROR(EINVAL));
        }

        return 0;
    }

    int gsws::scale(const uint8_t* const srcSlice[], const int srcStride[], int srcSliceY, int srcSliceH, uint8_t* const dst[], const int dstStride[])
    {
        LOCK();

        return sws_scale(swsctx_, srcSlice, srcStride, srcSliceY, srcSliceH, dst, dstStride);
    }
}//gff