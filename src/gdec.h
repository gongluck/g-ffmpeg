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
#include "gutil.h"

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

        /*
         * @brief   清理资源
         * @return  错误码
        */
        int cleanup() override;

        /*
         * @brief               设置解码参数
         * @return              错误码
         * @param par[in]       解码器参数
         * @param hwtype[in]    硬解类型
        */
        int copy_param(const AVCodecParameters* par, AVHWDeviceType hwtype = AV_HWDEVICE_TYPE_NONE);

        /*
         * @brief               解码一个AVPacket包
         * @return              错误码
         * @param packet[in]    数据包
         * @param frame[out]    结果AVFrame
        */
        int decode(std::shared_ptr<AVPacket> packet, std::shared_ptr<AVFrame> frame);

        /*
         * @brief               解码裸流数据
         * @return              错误码
         * @param data[in]      数据地址
         * @param size[in]      数据长度
         * @param frame[out]    结果AVFrame
         * @param len[out]      已经处理的数据长度
        */
        int decode(const void* data, uint32_t size, std::shared_ptr<AVFrame> frame, uint32_t& len);

    private:
        AVCodecContext* codectx_ = nullptr;
        AVCodecParserContext* par_ = nullptr;
        std::shared_ptr<AVPacket> pkt_ = GetPacket();
    };
}//gff

#endif//__GDEC_H__