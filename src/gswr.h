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

        /*
         * @brief   清理资源
         * @return  错误码
        */
        int cleanup() override;

        /*
         * @brief               设置重采样参数
         * @return              错误码
         * @param slayout[in]   输入通道布局
         * @param srate[in]     输入采样率
         * @param sfmt[in]      输入格式
         * @param dlayout[in]   输出通道布局
         * @param drate[in]     输出采样率
         * @param dfmt[in]      输出格式
        */
        int create_swr(int64_t slayout, int srate, enum AVSampleFormat sfmt,
            int64_t dlayout, int drate, enum AVSampleFormat dfmt);

        /*
         * @brief                   转换
         * @return                  成功返回转换样本数，否则返回错误码
         * @param srcSlice[in]      输入数据
         * @param srcStride[in]     输入数据大小(每通道in[index]的样本数)
         * @param dst[out]          输出数据
         * @param dstStride[out]    输出数据大小(每通道in[index]的样本数)
        */
        int convert(uint8_t** out, int out_count, const uint8_t** in, int in_count);

    private:
        SwrContext* swrctx_ = nullptr;
    };
}//gff

#endif//__GSWR_H__