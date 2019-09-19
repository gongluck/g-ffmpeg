/*******************************************************************
*  Copyright(c) 2019
*  All rights reserved.
*
*  文件名称:    gdemux.cpp
*  简要描述:    解封装
*
*  作者:  gongluck
*  说明:
*
*******************************************************************/

#include "gdemux.h"
#include "gutil.h"

namespace gff
{
    gdemux::~gdemux()
    {
        cleanup();
    }

    int gdemux::open(const char* in, const char* fmt/* = nullptr*/, const std::vector<std::pair<std::string, std::string>>& dicts/* = {}*/)
    {
        LOCK();
        CHECKSTOP();
        int ret = 0;

        cleanup();
        avdevice_register_all();

        if (fmt != nullptr && 
            (infmt_ = av_find_input_format(fmt)) == nullptr)
        {
            CHECKFFRET(AVERROR(EINVAL));
        }
        if (dicts.size() > 0)
        {
            for (const auto& p : dicts)
            {
                if (p.first.size() > 0 && p.second.size() > 0)
                {
                    ret = av_dict_set(&dict_, p.first.c_str(), p.second.c_str(), 0);
                    CHECKFFRET(ret);
                }
            }
        }

        fmtctx_ = avformat_alloc_context();
        if (fmtctx_ == nullptr)
        {
            ret = AVERROR(ENOMEM);
            CHECKFFRET(ret);
        }
        ret = avformat_open_input(&fmtctx_, in, infmt_, &dict_);
        CHECKFFRET(ret);

        ret = avformat_find_stream_info(fmtctx_, nullptr);
        CHECKFFRET(ret);

        av_dump_format(fmtctx_, 0, in, 0);

        getstatus() = WORKING;

        return ret;
    }

    int gdemux::readpacket(std::shared_ptr<AVPacket> packet)
    {
        LOCK();
        CHECKNOTSTOP();
        int ret = 0;

        // 解引用
        av_packet_unref(packet.get());
        
        return av_read_frame(fmtctx_, packet.get());
    }

    int gdemux::cleanup()
    {
        LOCK();

        avformat_close_input(&fmtctx_);
        av_dict_free(&dict_);
        infmt_ = nullptr;
        getstatus() = STOP;

        return 0;
    }

    int gdemux::get_steam_index(std::vector<unsigned int>& videovec, std::vector<unsigned int>& audiovec)
    {
        LOCK();
        CHECKNOTSTOP();

        if (fmtctx_ == nullptr)
        {
            CHECKFFRET(AVERROR(EINVAL));
        }
        videovec.clear();
        audiovec.clear();

        for (unsigned int i = 0; i < fmtctx_->nb_streams; ++i)
        {
            switch (fmtctx_->streams[i]->codecpar->codec_type)
            {
            case AVMEDIA_TYPE_VIDEO:
                videovec.push_back(i);
                break;
            case AVMEDIA_TYPE_AUDIO:
                audiovec.push_back(i);
                break;
            default:
                break;
            }
        }

        return 0;
    }

    int gdemux::get_stream_par(int index, const AVCodecParameters*& par)
    {
        LOCK();
        CHECKNOTSTOP();

        if (index < 0 || static_cast<unsigned int>(index) >= fmtctx_->nb_streams)
        {
            CHECKFFRET(AVERROR(EINVAL));
        }
        else
        {
            par = fmtctx_->streams[index]->codecpar;
        }

        return 0;
    }
}//gff