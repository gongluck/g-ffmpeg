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
#include <libavcodec/avcodec.h>

#ifdef __cplusplus
}
#endif

// 检查停止状态
#define CHECKSTOP() \
if(getstatus() != STOP)\
{\
    av_log(nullptr, AV_LOG_ERROR, "%s %d : %ld\n", __FILE__, __LINE__, AVERROR(EBUSY));\
    return AVERROR(EINVAL);\
}
#define CHECKNOTSTOP() \
if(getstatus() == STOP)\
{\
    av_log(nullptr, AV_LOG_ERROR, "%s %d : %ld\n", __FILE__, __LINE__, AVERROR(EINVAL));\
    return AVERROR(EINVAL);\
}

// 检查ffmpeg返回值
#define CHECKFFRET(ret) \
if (ret < 0)\
{\
    av_log(nullptr, AV_LOG_ERROR, "%s %d : %ld\n", __FILE__, __LINE__, ret);\
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
}//gff

#endif//__GUTIL_H__