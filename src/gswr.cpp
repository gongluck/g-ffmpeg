/*******************************************************************
*  Copyright(c) 2019
*  All rights reserved.
*
*  文件名称:    gswr.cpp
*  简要描述:    重采样
*
*  作者:  gongluck
*  说明:
*
*******************************************************************/

#include "gswr.h"
#include "gutil.h"

namespace gff
{     
    gswr::~gswr()
    {
        cleanup();
    }

    int gswr::cleanup()
    {
        LOCK();

        swr_free(&swrctx_);
        getstatus() = STOP;

        return 0;
    }

    int gswr::create_swr(int64_t slayout, int srate, enum AVSampleFormat sfmt,
        int64_t dlayout, int drate, enum AVSampleFormat dfmt)
    {
        LOCK();
        CHECKSTOP();

        swrctx_ = swr_alloc_set_opts(swrctx_, dlayout, dfmt, drate, slayout, sfmt, srate, 0, nullptr);
        if (swrctx_ == nullptr)
        {
            CHECKFFRET(AVERROR(EINVAL));
        }
        else
        {
            int ret = swr_init(swrctx_);
            CHECKFFRET(ret);
            getstatus() = WORKING;
        }

        return 0;
    }

    int gswr::convert(uint8_t** out, int out_count, const uint8_t** in, int in_count)
    {
        LOCK();
        CHECKNOTSTOP();

        if (swrctx_ == nullptr)
        {
            CHECKFFRET(AVERROR(EINVAL));
        }

        return swr_convert(swrctx_, out, out_count, in, in_count);
    }
}//gff