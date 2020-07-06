/*******************************************************************
*  Copyright(c) 2019
*  All rights reserved.
*
*  文件名称:    gmux.h
*  简要描述:    封装
*
*  作者:  gongluck
*  说明:
*
*******************************************************************/

#ifndef __GMUX_H__
#define __GMUX_H__

#include "gavbase.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include <libavformat/avformat.h>

#ifdef __cplusplus
}
#endif

namespace gff
{
    class gmux : public gavbase
    {
    public:
        ~gmux();

        /*
         * @brief   清理资源
         * @return  错误码
        */
        int cleanup() override;

        /*
         * @brief           创建输出
         * @return          错误码
         * @param out[in]   输出uri
        */
        int create_output(const char* out);

        /*
         * @brief               创建输出
         * @return              错误码
         * @param codectx[in]   编码上下文
         * @param index[out]    流索引
        */
        int create_stream(const AVCodecContext* codectx, int& index);

        /*
         * @brief               写头
         * @return              错误码
        */
        int write_header();

        /*
         * @brief               获取时基
         * @return              错误码
         * @param index[in]     流索引
         * @param timebase[out] 时基
        */
        int get_timebase(int index, AVRational& timebase);

        /*
         * @brief               写帧
         * @return              错误码
         * @param packet[in]    帧
        */
        int write_packet(std::shared_ptr<AVPacket> packet);

    private:
        AVFormatContext* fmt_ = nullptr;
    };
}//gff

#endif//__GMUX_H__