/*******************************************************************
*  Copyright(c) 2019
*  All rights reserved.
*
*  文件名称:    gavbase.h
*  简要描述:    通用基类
*
*  作者:  gongluck
*  说明:
*
*******************************************************************/

#ifndef __GAVBASE_H__
#define __GAVBASE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <libavutil/opt.h>

#ifdef __cplusplus
}
#endif

#include <mutex>

namespace gff
{
    typedef std::recursive_mutex gmutex;
    class gavbase
    {
    public:
        virtual ~gavbase() = default;

        // 获取锁
        inline gmutex& getmutex()
        {
            return mutex_;
        }

        // 检查停止状态
        inline bool checkstop() const
        {
            if (this->status_ != STOP)
            {
                av_log(nullptr, AV_LOG_ERROR, "%s %d : %ld\n", __FILE__, __LINE__, AVERROR(EINVAL));
                return false;
            }
            else
            {
                return true;
            }
        }
        inline bool checknotstop() const
        {
            if (this->status_ == STOP)
            {
                av_log(nullptr, AV_LOG_ERROR, "%s %d : %ld\n", __FILE__, __LINE__, AVERROR(EINVAL));
                return false;
            }
            else
            {
                return true;
            }
        }

        // 检查ffmpeg返回值
        inline bool checkret(int ret) const
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

    private:
        // 状态
        enum STATUS { STOP, WORKING }status_ = STOP;

        // 操作锁
        gmutex mutex_;
    };// gavbase

    // 锁管理
    class glock
    {
    public:
        glock(gmutex& mutex) : mutex_(mutex)
        {
            mutex_.lock();
        }
        ~glock()
        {
            mutex_.unlock();
        }

    private:
        gmutex& mutex_;
    };// glock
}// gff

#endif//__GAVBASE_H__