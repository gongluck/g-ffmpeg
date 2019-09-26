/*******************************************************************
*  Copyright(c) 2019
*  All rights reserved.
*
*  文件名称:    gutil.cpp
*  简要描述:    通用工具
*
*  作者:  gongluck
*  说明:
*
*******************************************************************/

#include "gutil.h"

namespace gff
{
    AVPacket* CreatePacket()
    {
        auto p = av_packet_alloc();
        av_init_packet(p);
        return p;
    }
    void FreePacket(AVPacket* p)
    {
        av_packet_unref(p);
        av_packet_free(&p);
    }
    std::shared_ptr<AVPacket> GetPacket()
    {
        return std::shared_ptr<AVPacket>(CreatePacket(), FreePacket);
    }

    AVFrame* CreateFrame()
    {
        auto p = av_frame_alloc();
        return p;
    }
    void FreeFrame(AVFrame* p)
    {
        av_frame_unref(p);
        av_frame_free(&p);
    }
    std::shared_ptr<AVFrame> GetFrame()
    {
        return std::shared_ptr<AVFrame>(CreateFrame(), FreeFrame);
    }

    int GetFrameBuf(std::shared_ptr<AVFrame> frame, int w, int h, AVPixelFormat fmt, int align)
    {
        if (frame == nullptr)
        {
            CHECKFFRET(AVERROR(EINVAL));
        }

        frame->width = w;
        frame->height = h;
        frame->format = fmt;
        
        return av_frame_get_buffer(frame.get(), align);
    }

    int GetFrameBuf(std::shared_ptr<AVFrame> frame, int samples, uint64_t layout, AVSampleFormat fmt, int align)
    {
        if (frame == nullptr)
        {
            CHECKFFRET(AVERROR(EINVAL));
        }

        frame->nb_samples = samples;
        frame->channel_layout = layout;
        frame->format = fmt;

        return av_frame_get_buffer(frame.get(), align);
    }

    int frame_make_writable(std::shared_ptr<AVFrame> frame)
    {
        if (frame == nullptr)
        {
            CHECKFFRET(AVERROR(EINVAL));
        }

        return av_frame_make_writable(frame.get());
    }
}//gff