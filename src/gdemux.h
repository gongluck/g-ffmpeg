/*******************************************************************
*  Copyright(c) 2019
*  All rights reserved.
*
*  文件名称:    gdemux.h
*  简要描述:    解封装
*
*  作者:  gongluck
*  说明:
*
*******************************************************************/

#ifndef __GDEMUX_H__
#define __GDEMUX_H__

#include "gavbase.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>

#ifdef __cplusplus
}
#endif

#include <vector>

namespace gff
{
    class gdemux : public gavbase
    {
    public:
        ~gdemux();

        /*
         * @brief                   打开输入
         * @return                  错误码
         * @param in[in]            输入URI
         * @param fmt[in]           格式
         * @param dicts[in]         自定义参数键值对
         * @param read_packet[in]   自定义输入回调
         * @param opaque[in]        自定义输入回调的用户参数
         * @param bufsize[in]       avio缓冲区大小
        */
        int open(const char* in, const char* fmt = nullptr, const std::vector<std::pair<std::string, std::string>>& dicts = {},
            int (*read_packet)(void* opaque, uint8_t* buf, int buf_size) = nullptr, void* opaque = nullptr, size_t bufsize = 1024);

        /*
         * @brief               读取一个AVPacket
         * @return              错误码
         * @param packet[out]   接收数据包
        */
        int readpacket(std::shared_ptr<AVPacket> packet);

        /*
         * @brief   清理资源
         * @return  错误码
        */
        int cleanup() override;

        /*
         * @brief               获取流索引
         * @return              错误码
         * @param videovec[out] 接收视频流索引
         * @param audiovec[out] 接收音频流索引
        */
        int get_steam_index(std::vector<unsigned int>& videovec, std::vector<unsigned int>& audiovec);

        /*
         * @brief               获取流参数
         * @return              错误码
         * @param index[in]     流索引
         * @param par[in out]   接收流参数
         * @param timebase[out] 接收时基
        */
        int get_stream_par(unsigned int index, const AVCodecParameters*& par, AVRational& timebase);

        /*
         * @brief                   跳转
         * @return                  错误码
         * @param index[in]         流索引
         * @param timestamp[in]     目标时间戳
         * @param seekanyframe[in]  跳转到任意帧
        */
        int seek_frame(int index, int64_t timestamp, bool seekanyframe = false);

        /*
         * @brief                   获取时长
         * @return                  错误码
         * @param duration[out]     接收时长
         * @param timebase[in]      时基
        */
        int get_duration(int64_t& duration, AVRational timebase = {1,1});

    private:
        AVFormatContext* fmtctx_ = nullptr;
        AVInputFormat* infmt_ = nullptr;
        AVDictionary* dict_ = nullptr;
    };
}//gff

#endif//__GDEMUX_H__
