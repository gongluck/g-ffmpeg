/*******************************************************************
*  Copyright(c) 2019
*  All rights reserved.
*
*  文件名称:    gdec.cpp
*  简要描述:    解码
*
*  作者:  gongluck
*  说明:
*
*******************************************************************/

#include "gdec.h"
#include "gutil.h"

namespace gff
{
    gdec::~gdec()
    {
        cleanup();
    }

    int gdec::cleanup()
    {
        LOCK();

        avcodec_free_context(&codectx_);
        getstatus() = STOP;

        return 0;
    }

    int gdec::copy_param(const AVCodecParameters* par)
    {
        LOCK();
        CHECKSTOP();
        int ret = 0;

        cleanup();

        auto codec = avcodec_find_decoder(par->codec_id);
        if (codec == nullptr)
        {
            CHECKFFRET(AVERROR(EINVAL));
        }
        codectx_ = avcodec_alloc_context3(codec);
        if (codectx_ == nullptr)
        {
            CHECKFFRET(AVERROR(ENOMEM));
        }

        ret = avcodec_parameters_to_context(codectx_, par);
        CHECKFFRET(ret);
        ret = avcodec_open2(codectx_, codec, nullptr);
        CHECKFFRET(ret);

        getstatus() = WORKING;

        return 0;
    }

    int gdec::decode(const AVPacket* packet, AVFrame& frame)
    {
        LOCK();
        CHECKNOTSTOP();
        int ret = 0;

        if (codectx_ == nullptr)
        {
            CHECKFFRET(AVERROR(EINVAL));
        }

        if (packet != nullptr)
        {
            // 发送将要解码的数据
            ret = avcodec_send_packet(codectx_, packet);
            CHECKFFRET(ret);
        }
        
        // 接收解码数据
        return avcodec_receive_frame(codectx_, &frame);
    }
}//gff