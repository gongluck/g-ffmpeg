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

        // 销毁资源
        int cleanup();

        // 创建输出
        int create_output(const char* out);

        // 创建流
        int create_stream(const AVCodecContext* codectx, int& index);

        // 写文件头
        int write_header();

        // 获取流时基
        int get_timebase(int index, AVRational& timebase);

        // 写帧
        int write_packet(AVPacket& packet);

    private:
        AVFormatContext* fmt_ = nullptr;
    };
}//gff

#endif//__GMUX_H__