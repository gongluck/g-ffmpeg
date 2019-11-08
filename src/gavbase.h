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

#include <memory>
#include <mutex>

namespace gff
{
    typedef std::recursive_mutex gmutex;
    typedef enum STATUS { STOP, WORKING } STATUS;
    class gavbase
    {
    public:
        virtual ~gavbase() = default;

    protected:
        // 获取锁
        inline gmutex& getmutex()
        {
            return mutex_;
        } 

        // 获取状态
        inline STATUS getstatus() const
        {
            return status_;
        }
        inline STATUS& getstatus()
        {
            return status_;
        }

        // 清理
        virtual int cleanup() = 0;

    private:
        // 状态
        STATUS status_ = STOP;

        // 操作锁
        gmutex mutex_;
    };// gavbase
}// gff

#endif//__GAVBASE_H__
