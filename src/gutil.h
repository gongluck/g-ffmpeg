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

#ifdef __cplusplus
}
#endif

namespace gff
{
// 检查停止状态
#define CHECKSTOP() \
{\
        if (this->status_ != STOP)\
        {\
            av_log(nullptr, AV_LOG_ERROR, "%s %d : %ld\n", __FILE__, __LINE__, AVERROR(EBUSY));\
            return AVERROR(EBUSY);\
        }\
}
#define CHECKNOTSTOP() \
    {\
        if (this->status_ == STOP)\
        {\
            av_log(nullptr, AV_LOG_ERROR, "%s %d : %ld\n", __FILE__, __LINE__, AVERROR(EINVAL));\
            return AVERROR(EINVAL);\
        }\
    }

    // 检查ffmpeg返回值
#define CHECKFFRET(ret) \
    {\
        if (ret < 0)\
        {\
            av_log(nullptr, ret != AVERROR(EAGAIN) ? (ret != AVERROR_EOF ? AV_LOG_ERROR : AV_LOG_INFO) : AV_LOG_DEBUG, "%s %d : %ld\n", __FILE__, __LINE__, ret);\
            return ret;\
        }\
    }
}

#endif//__GUTIL_H__