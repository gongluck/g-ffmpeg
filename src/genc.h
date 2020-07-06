/*******************************************************************
*  Copyright(c) 2019
*  All rights reserved.
*
*  文件名称:    genc.h
*  简要描述:    编码
*
*  作者:  gongluck
*  说明:
*
*******************************************************************/

#ifndef __GENC_H__
#define __GENC_H__

#include "gavbase.h"

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
    class genc : public gavbase
    {
    public:
        ~genc();

        /*
         * @brief   清理资源
         * @return  错误码
        */
        int cleanup() override;

        /*
         * @brief                   设置视频编码参数
         * @return                  错误码
         * @param codecname[in]     编码器名称
         * @param bitrate[in]       比特率
         * @param width[in]         视频宽
         * @param height[in]        视频高
         * @param framerate[in]     帧率
         * @param gop[in]           gop
         * @param maxbframes[in]    最大B帧数
         * @param fmt               输入帧格式
        */
        int set_video_param(const char* codecname, int64_t bitrate, int width, int height, AVRational timebase, AVRational framerate, int gop, int maxbframes, AVPixelFormat fmt);
        
        /*
         * @brief                   设置音频编码参数
         * @return                  错误码
         * @param codecname[in]     编码器名称
         * @param bitrate[in]       比特率
         * @param samplerate[in]    采样频率
         * @param channellayout[in] 通道格式
         * @param channels[in]      通道数
         * @param fmt               输入帧格式
         * @param framesize[out]    每个通道的样本数
        */
        int set_audio_param(const char* codecname, int64_t bitrate, int samplerate, uint64_t channellayout, int channels, AVSampleFormat fmt, int& framesize);

        /*
         * @brief                       获取流参数
         * @return                      错误码
         * @param AVCodecContext[out]   流参数
        */
        int get_codectx(const AVCodecContext*& codectx);

        /*
         * @brief           输入编码
         * @return          错误码
         * @param frame[in] 输入帧
        */
        int encode_push_frame(std::shared_ptr<AVFrame> frame);

        /*
         * @brief               获取编码
         * @return              错误码
         * @param frame[out]    输出帧
        */
        int encode_get_packet(std::shared_ptr<AVPacket> packet);

    private:
        AVCodecContext* codectx_ = nullptr;
    };
}//gff

#endif//__GENC_H__