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

        // 打开输入
        int open(const char* in, const char* fmt = nullptr, const std::vector<std::pair<std::string, std::string>>& dicts = {});

        // 读取一个AVPacket
        int readpacket(std::shared_ptr<AVPacket> packet);

        // 清理资源
        int cleanup() override;

        // 获取流索引
        int get_steam_index(std::vector<unsigned int>& videovec, std::vector<unsigned int>& audiovec);

        // 获取流参数
        int get_stream_par(int index, const AVCodecParameters*& par, AVRational& timebase);

        // 跳转
        int seek_frame(int index, int64_t timestamp, bool seekanyframe = false);

    private:
        AVFormatContext* fmtctx_ = nullptr;
        AVInputFormat* infmt_ = nullptr;
        AVDictionary* dict_ = nullptr;
    };
}//gff

#endif//__GDEMUX_H__