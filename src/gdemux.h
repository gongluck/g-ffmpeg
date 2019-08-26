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
        int open(const char* in);

        // 读取一个AVPacket
        int readpacket(AVPacket& out);

        // 关闭输入
        int close();

        // 获取流索引
        int get_steam_index(std::vector<unsigned int>& videovec, std::vector<unsigned int>& audiovec);

        // 获取流参数
        int get_stream_par(int index, const AVCodecParameters*& par);

    private:
        AVFormatContext* fmtctx_ = nullptr;
    };
}//gff

#endif//__GDEMUX_H__