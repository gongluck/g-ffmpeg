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

    private:
        // 状态
        enum STATUS { STOP, WORKING }status_ = STOP;

        // 操作锁
        gmutex mutex_;
    };// gavbase
}// gff

#endif//__GAVBASE_H__