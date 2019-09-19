/*******************************************************************
*  Copyright(c) 2019
*  All rights reserved.
*
*  文件名称:    gmux.cpp
*  简要描述:    输出
*
*  作者:  gongluck
*  说明:
*
*******************************************************************/

#include "gmux.h"
#include "gutil.h"

namespace gff
{
    gmux::~gmux()
    {
        cleanup();
    }

    int gmux::cleanup()
    {
        LOCK();

        if (fmt_ != nullptr)
        {
            int ret = av_write_trailer(fmt_);
            CHECKFFRET(ret);
            ret = avio_closep(&fmt_->pb);
            CHECKFFRET(ret);
            av_dump_format(fmt_, 0, fmt_->url, 1);
            avformat_free_context(fmt_);
            fmt_ = nullptr;
        }

        getstatus() = STOP;

        return 0;
    }

    int gmux::create_output(const char* out)
    {
        LOCK();
        CHECKSTOP();

        cleanup();
        int ret = avformat_alloc_output_context2(&fmt_, nullptr, nullptr, out);
        CHECKFFRET(ret);

        getstatus() = WORKING;

        return 0;
    }

    int gmux::create_stream(const AVCodecContext* codectx, int& index)
    {
        LOCK();
        CHECKNOTSTOP();

        if (fmt_ == nullptr || codectx == nullptr)
        {
            CHECKFFRET(AVERROR(EINVAL));
        }

        auto codec = avcodec_find_encoder(codectx->codec_id);
        if (codec == nullptr)
        {
            CHECKFFRET(AVERROR(EINVAL));
        }

        auto stream = avformat_new_stream(fmt_, codec);
        if (stream == nullptr)
        {
            CHECKFFRET(AVERROR(ENOMEM));
        }

        int ret = avcodec_parameters_from_context(fmt_->streams[stream->index]->codecpar, codectx);
        CHECKFFRET(ret);
        index = stream->index;
                
        return 0;
    }

    int gmux::write_header()
    {
        LOCK();
        CHECKNOTSTOP();

        if (fmt_ == nullptr)
        {
            CHECKFFRET(AVERROR(EINVAL));
        }

        int ret = avio_open2(&fmt_->pb, fmt_->url, AVIO_FLAG_WRITE, nullptr, nullptr);
        CHECKFFRET(ret);

        av_dump_format(fmt_, 0, fmt_->url, 1);

        ret = avformat_write_header(fmt_, nullptr);
        CHECKFFRET(ret);
       
        return 0;
    }

    int gmux::get_timebase(int index, AVRational& timebase)
    {
        LOCK();
        CHECKNOTSTOP();

        if (fmt_ == nullptr || index < 0 || index >= static_cast<int>(fmt_->nb_streams))
        {
            CHECKFFRET(AVERROR(EINVAL));
        }

        timebase = fmt_->streams[index]->time_base;

        return 0;
    }

    int gmux::write_packet(std::shared_ptr<AVPacket> packet)
    {
        LOCK();
        CHECKNOTSTOP();

        if (fmt_ == nullptr)
        {
            CHECKFFRET(AVERROR(EINVAL));
        }

        return av_interleaved_write_frame(fmt_, packet.get());
    }
}//gff