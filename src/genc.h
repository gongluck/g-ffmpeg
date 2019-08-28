/*******************************************************************
*  Copyright(c) 2019
*  All rights reserved.
*
*  文件名称:    genc.h
*  简要描述:    编码
*
*  作者:  gongluck
*  说明:
*
*******************************************************************/

#ifndef __GENC_H__
#define __GENC_H__

#include "gavbase.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include <libavcodec/avcodec.h>

#ifdef __cplusplus
}
#endif

namespace gff
{
    class genc : public gavbase
    {
    public:
        ~genc();

        // 销毁资源
        int cleanup();

        // 设置视频参数
        int set_video_param(const char* codecname, int64_t bitrate, int width, int height, AVRational timebase, AVRational framerate, int gop, int maxbframes, AVPixelFormat fmt);
        // 设置音频参数
        int set_audio_param(const char* codecname, int64_t bitrate, int samplerate, uint64_t channellayout, int channels, AVSampleFormat fmt, int& framesize);

        // 编码
        int encode_push_frame(const AVFrame* frame);
        int encode_get_packet(AVPacket& packet);

    private:
        AVCodecContext* codectx_ = nullptr;
    };
}//gff

#endif//__GENC_H__