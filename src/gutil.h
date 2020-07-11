/*******************************************************************
*  Copyright(c) 2019
*  All rights reserved.
*
*  文件名称:    gutil.h
*  简要描述:    通用工具
*
*  作者:  gongluck
*  说明:
*
*******************************************************************/

#ifndef __GUTIL_H__
#define __GUTIL_H__

#include "gavbase.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include <libavutil/opt.h>
#include <libavutil/audio_fifo.h>
#include <libavcodec/avcodec.h>

#ifdef __cplusplus
}
#endif

// 检查停止状态
#define CHECKSTOP() \
if(getstatus() != STOP)\
{\
    av_log(nullptr, AV_LOG_ERROR, "%s %d : %d\n", __FILE__, __LINE__, AVERROR(EBUSY));\
    return AVERROR(EINVAL);\
}
#define CHECKNOTSTOP() \
if(getstatus() == STOP)\
{\
    av_log(nullptr, AV_LOG_ERROR, "%s %d : %d\n", __FILE__, __LINE__, AVERROR(EINVAL));\
    return AVERROR(EINVAL);\
}

// 检查ffmpeg返回值
#ifdef av_err2str
#undef av_err2str
#endif
static char av_error[AV_ERROR_MAX_STRING_SIZE] = { 0 };
#define av_err2str(errnum) \
    av_make_error_string(av_error, AV_ERROR_MAX_STRING_SIZE, errnum)

#define CHECKFFRET(ret) \
if (ret < 0 && ret != AVERROR(EAGAIN))\
{\
    av_log(nullptr, AV_LOG_ERROR, "%s %d : %d %s\n", __FILE__, __LINE__, ret, av_err2str(ret));\
    return ret;\
}

// 锁
#define LOCK() std::lock_guard<decltype(getmutex())> _lock(getmutex())

#include <iostream>
namespace gff
{
    // 获取AVPacet
    std::shared_ptr<AVPacket> GetPacket();
    // 获取AVFrame
    std::shared_ptr<AVFrame> GetFrame();

    // 分配AVFrame数据空间
    int GetFrameBuf(std::shared_ptr<AVFrame> frame, int w, int h, AVPixelFormat fmt, int align);
    int GetFrameBuf(std::shared_ptr<AVFrame> frame, int samples, uint64_t layout, AVSampleFormat fmt, int align);

    // 确保能写frame
    int frame_make_writable(std::shared_ptr<AVFrame> frame);

    // 获取硬解码数据
    int hwframe_to_frame(std::shared_ptr<AVFrame> hwframe, std::shared_ptr<AVFrame> frame);
}//gff

#endif//__GUTIL_H__
