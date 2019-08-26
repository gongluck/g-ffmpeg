/*******************************************************************
*  Copyright(c) 2019
*  All rights reserved.
*
*  文件名称:    gdec.h
*  简要描述:    解码
*
*  作者:  gongluck
*  说明:
*
*******************************************************************/

#ifndef __GDEC_H__
#define __GDEC_H__

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
    class gdec : public gavbase
    {
    public:
        ~gdec();

        // 销毁资源
        int cleanup();

        // 设置解码参数
        int copy_param(const AVCodecParameters* par);

        // 解码
        int decode(const AVPacket* packet, AVFrame& frame);

    private:
        AVCodecContext* codectx_ = nullptr;
    };
}//gff

#endif//__GDEC_H__