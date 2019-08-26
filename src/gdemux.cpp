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
        close();
    }

    int gdemux::open(const char* in)
    {
        LOCK();
        CHECKSTOP();
        int ret = 0;

        close();
        fmtctx_ = avformat_alloc_context();
        if (fmtctx_ == nullptr)
        {
            ret = AVERROR(ENOMEM);
            CHECKFFRET(ret);
        }
        ret = avformat_open_input(&fmtctx_, in, nullptr, nullptr);
        CHECKFFRET(ret);

        ret = avformat_find_stream_info(fmtctx_, nullptr);
        CHECKFFRET(ret);

        av_dump_format(fmtctx_, 0, in, 0);

        getstatus() = WORKING;

        return ret;
    }

    int gdemux::readpacket(AVPacket& out)
    {
        LOCK();
        CHECKNOTSTOP();

        // 读数据
        return av_read_frame(fmtctx_, &out);
    }

    int gdemux::close()
    {
        LOCK();
        //CHECKNOTSTOP();

        avformat_close_input(&fmtctx_);

        getstatus() = STOP;

        return 0;
    }

    int gdemux::get_steam_index(std::vector<int>& videovec, std::vector<int>& audiovec)
    {
        LOCK();
        if (fmtctx_ == nullptr)
        {
            CHECKFFRET(AVERROR(EINVAL));
        }
        videovec.clear();
        audiovec.clear();

        for (int i = 0; i < fmtctx_->nb_streams; ++i)
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