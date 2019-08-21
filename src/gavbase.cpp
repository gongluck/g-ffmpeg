/*******************************************************************
*  Copyright(c) 2019
*  All rights reserved.
*
*  文件名称:    gavbase.cpp
*  简要描述:    通用基类
*
*  作者:  gongluck
*  说明:
*
*******************************************************************/

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
    // 获取锁
    inline std::recursive_mutex& gavbase::getmutex()
    {
        return mutex_;
    }

    // 检查停止状态
    inline int gavbase::checkstop() const
    {
        if (this->status_ != STOP)
        {
            av_log(nullptr, AV_LOG_ERROR, "%s %d : %ld\n", __FILE__, __LINE__, AVERROR(EINVAL));
            return AVERROR(EINVAL);
        }
    }
    inline int gavbase::checknotstop() const
    {
        if (this->status_ == STOP)
        {
            av_log(nullptr, AV_LOG_ERROR, "%s %d : %ld\n", __FILE__, __LINE__, AVERROR(EINVAL));
            return AVERROR(EINVAL);
        }
    }

    // 检查ffmpeg返回值
    inline bool gavbase::checkret(int ret) const
    {
        if (ret < 0)
        {
            av_log(nullptr, ret != AVERROR(EAGAIN) ? (ret != AVERROR_EOF ? AV_LOG_ERROR : AV_LOG_INFO) : AV_LOG_DEBUG, "%s %d : %ld\n", __FILE__, __LINE__, ret);
            return false;
        }
        else
        {
            return true;
        }
    }
}// gff