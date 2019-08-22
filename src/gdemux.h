/*******************************************************************
*  Copyright(c) 2019
*  All rights reserved.
*
*  文件名称:    gdemux.h
*  简要描述:    解封装
*
*  作者:  gongluck
*  说明:
*
*******************************************************************/

#ifndef __GDEMUX_H__
#define __GDEMUX_H__

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
    class gdemux : public gavbase
    {
    public:
        ~gdemux();
        int openinput(const char* in);
        int readpacket(AVPacket& out);
        int stop();

    private:
        AVFormatContext* fmtctx_ = nullptr;
    };
}//gff

#endif//__GDEMUX_H__