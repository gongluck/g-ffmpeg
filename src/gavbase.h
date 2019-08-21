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

#include <mutex>

namespace gff
{
    class gavbase
    {
    public:
        virtual ~gavbase() = default;

        // 获取锁
        inline std::recursive_mutex& getmutex();

        // 检查停止状态
        inline int checkstop() const;
        inline int checknotstop() const;

        // 检查ffmpeg返回值
        inline bool checkret(int ret) const;

    private:
        // 状态
        enum STATUS { STOP, WORKING }status_ = STOP;

        // 操作锁
        std::recursive_mutex mutex_;
        bool locked_ = false;
    };// gavbase

    // 锁管理
    class glock
    {
    public:
        glock(std::recursive_mutex& mutex) : mutex_(mutex)
        {
            mutex_.lock();
        }
        ~glock()
        {
            mutex_.unlock();
        }

    private:
        std::recursive_mutex& mutex_;
    };// glock
}// gff

#endif//__GAVBASE_H__