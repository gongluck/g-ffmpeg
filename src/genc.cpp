/*******************************************************************
*  Copyright(c) 2019
*  All rights reserved.
*
*  文件名称:    genc.cpp
*  简要描述:    编码
*
*  作者:  gongluck
*  说明:
*
*******************************************************************/

#include "genc.h"
#include "gutil.h"

namespace gff
{
    genc::~genc()
    {
        cleanup();
    }

    int genc::cleanup()
    {
        LOCK();

        avcodec_free_context(&codectx_);

        return 0;
    }

    int genc::set_video_param(const char* codecname, int64_t bitrate, int width, int height, AVRational timebase, AVRational framerate, int gop, int maxbframes, AVPixelFormat fmt)
    {
        LOCK();

        auto codec = avcodec_find_encoder_by_name(codecname);
        if (codec == nullptr)
        {
            CHECKFFRET(AVERROR(EINVAL));
        }

        cleanup();
        codectx_ = avcodec_alloc_context3(codec);
        if (codectx_ == nullptr)
        {
            CHECKFFRET(AVERROR(ENOMEM));
        }

        codectx_->bit_rate = bitrate;
        codectx_->width = width;
        codectx_->height = height;
        codectx_->time_base = timebase;
        codectx_->framerate = framerate;
        codectx_->gop_size = gop;
        codectx_->max_b_frames = maxbframes;
        codectx_->pix_fmt = fmt;
        codectx_->codec_type = AVMEDIA_TYPE_VIDEO;

        return avcodec_open2(codectx_, codec, nullptr);
    }

    int genc::set_audio_param(const char* codecname, int64_t bitrate, int samplerate, uint64_t channellayout, int channels, AVSampleFormat fmt, int& framesize)
    {
        LOCK();
        int ret = 0;

        auto codec = avcodec_find_encoder_by_name(codecname);
        if (codec == nullptr)
        {
            CHECKFFRET(AVERROR(EINVAL));
        }

        cleanup();
        codectx_ = avcodec_alloc_context3(codec);
        if (codectx_ == nullptr)
        {
            CHECKFFRET(AVERROR(ENOMEM));
        }

        codectx_->bit_rate = bitrate;
        codectx_->sample_rate = samplerate;
        codectx_->channel_layout = channellayout;
        codectx_->channels = channels;
        codectx_->sample_fmt = fmt;
        codectx_->codec_type = AVMEDIA_TYPE_AUDIO;

        ret = avcodec_open2(codectx_, codec, nullptr);
        CHECKFFRET(ret);
        framesize = codectx_->frame_size;

        return ret;
    }

    int genc::encode_push_frame(const AVFrame* frame)
    {
        LOCK();

        if (codectx_ == nullptr)
        {
            CHECKFFRET(AVERROR(EINVAL));
        }

        return avcodec_send_frame(codectx_, frame);
    }

    int genc::encode_get_packet(AVPacket& packet)
    {
        LOCK();

        if (codectx_ == nullptr)
        {
            CHECKFFRET(AVERROR(EINVAL));
        }

        return avcodec_receive_packet(codectx_, &packet);
    }
}//gff