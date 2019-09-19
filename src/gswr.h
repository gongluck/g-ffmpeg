/*******************************************************************
*  Copyright(c) 2019
*  All rights reserved.
*
*  文件名称:    gswr.h
*  简要描述:    重采样
*
*  作者:  gongluck
*  说明:
*
*******************************************************************/

#ifndef __GSWR_H__
#define __GSWR_H__

#include "gavbase.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include <libswresample/swresample.h>

#ifdef __cplusplus
}
#endif

namespace gff
{
    class gswr : public gavbase
    {
    public:
        ~gswr();

        // 销毁资源
        int cleanup() override;

        // 设置参数
        int create_swr(int64_t slayout, int srate, enum AVSampleFormat sfmt,
            int64_t dlayout, int drate, enum AVSampleFormat dfmt);

        // 转换
        int convert(uint8_t** out, int out_count, const uint8_t** in, int in_count);

    private:
        SwrContext* swrctx_ = nullptr;
    };
}//gff

#endif//__GSWR_H__