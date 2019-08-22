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
        stop();
    }

    int gdemux::openinput(const char* in)
    {
        LOCK();
        CHECKSTOP();
        int ret = 0;

        stop();
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

    int gdemux::stop()
    {
        LOCK();
        //CHECKNOTSTOP();

        avformat_close_input(&fmtctx_);

        getstatus() = STOP;

        return 0;
    }
}