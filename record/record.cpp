/*******************************************************************
*  Copyright(c) 2019
*  All rights reserved.
*
*  文件名称:    record.cpp
*  简要描述:    测试录屏
*
*  作者:  gongluck
*  说明:
*
*******************************************************************/

#include <iostream>
#include <queue>
#include "../src/gutil.h"
#include "../src/gdemux.h"
#include "../src/gdec.h"
#include "../src/gsws.h"
#include "../src/genc.h"
#include "../src/gmux.h"

const int FPS = 30;
const char* STRFPS = "30";

int main(int argc, char* argv[])
{
    int ret = 0;
    bool bstop = false;
    // 流索引
    std::vector<unsigned int> videovec, audiovec;
    int vindex = -1;
    int ovindex = -1;
    // 流参数
    const AVCodecParameters* vpar = nullptr;
    const AVCodecContext* vcodectx = nullptr;
    AVRational vtimebase;
    AVRational ovtimebase;
    // 帧队列
    std::queue<std::shared_ptr<AVPacket>> packet_queue;
    std::mutex packet_queue_mutex;
    std::queue<std::shared_ptr<AVFrame>> vframe_queue;
    std::mutex vframe_queue_mutex;
    std::queue<std::shared_ptr<AVPacket>> opacket_queue;
    std::mutex opacket_queue_mutex;
    // 视频解封装
    gff::gdemux demux_desktop;
    ret = demux_desktop.open("desktop", "gdigrab", { {"framerate", STRFPS} });
    CHECKFFRET(ret);
    ret = demux_desktop.get_steam_index(videovec, audiovec);
    CHECKFFRET(ret);
    vindex = videovec.size() > 0 ? videovec.at(0) : -1;
    ret = demux_desktop.get_stream_par(vindex, vpar, vtimebase);
    CHECKFFRET(ret);
    // 视频解封装线程
    std::thread demux_desktop_thread([&]()
    {
        int64_t startpts = -1;
        decltype(gff::GetPacket()) packet = nullptr;
        do
        {
            packet = gff::GetPacket();
            if (packet == nullptr)
            {
                CHECKFFRET(AVERROR(ENOMEM));
            }

            ret = demux_desktop.readpacket(packet);
            CHECKFFRET(ret);

            if (packet != nullptr)
            {
                if (startpts == -1)
                {
                    startpts = packet->pts;
                }
                packet->pts -= startpts;

                /*std::cout << "got a packet, index " << packet->stream_index << " pts " <<
                    av_rescale_q(packet->pts, vtimebase, { 1,1 }) << std::endl;*/
                
                std::lock_guard<decltype(packet_queue_mutex)> _lock(packet_queue_mutex);
                if (packet_queue.size() > 100)
                {
                    std::queue<std::shared_ptr<AVPacket>> empty;
                    packet_queue.swap(empty);
                    std::cout << "too many packets" << std::endl;
                }

                packet_queue.push(packet);
            }

        } while (ret == 0 && !bstop);

        std::cout << "exit demux_desktop_thread" << std::endl;
        return 0;
    }
    );

    // 视频解码
    gff::gdec vdec;
    ret = vdec.copy_param(vpar);
    CHECKFFRET(ret);
    // 视频解码线程
    std::thread vdec_thread([&]()
    {
        int ret = 0;
        decltype(gff::GetPacket()) packet = nullptr;
        decltype(gff::GetFrame()) frame = nullptr;
        gff::gsws sws;
        bool bcreated = false;
        do
        {
            if (frame != nullptr)
            {
                decltype(gff::GetFrame()) pushframe = nullptr;
                if (frame->format != AV_PIX_FMT_NV12)
                {
                    if (!bcreated)
                    {
                        ret = sws.create_sws(static_cast<AVPixelFormat>(frame->format), frame->width, frame->height,
                            AV_PIX_FMT_NV12, frame->width, frame->height);
                        CHECKFFRET(ret);
                        bcreated = true;
                    }
                    auto nvframe = gff::GetFrame();
                    if (nvframe == nullptr)
                    {
                        CHECKFFRET(AVERROR(ENOMEM));
                    }
                    ret = gff::GetFrameBuf(nvframe, frame->width, frame->height, AV_PIX_FMT_NV12, 1);
                    CHECKFFRET(ret);
                    ret = sws.scale(frame->data, frame->linesize, 0, frame->height, nvframe->data, nvframe->linesize);
                    CHECKFFRET(ret);

                    pushframe = nvframe;
                }
                else
                {
                    pushframe = frame;
                }

                pushframe->pts = frame->pts;

                {
                    std::lock_guard<decltype(vframe_queue_mutex)> _lock(vframe_queue_mutex);
                    if (vframe_queue.size() > 100)
                    {
                        std::queue<std::shared_ptr<AVFrame>> empty;
                        vframe_queue.swap(empty);
                        std::cout << "too many vframes" << std::endl;
                    }
                    vframe_queue.push(pushframe);
                }
                
                /*std::cout << "got a vframe, pts " <<
                    av_rescale_q(frame->pts, vtimebase, { 1,1 }) << std::endl;*/
            }

            frame = gff::GetFrame();
            if (frame == nullptr)
            {
                CHECKFFRET(AVERROR(ENOMEM));
            }

            {
                std::lock_guard<decltype(packet_queue_mutex)> _lock(packet_queue_mutex);
                if (packet_queue.size() > 0)
                {
                    packet = packet_queue.front();
                    packet_queue.pop();
                }
            }

            ret = vdec.decode(packet, frame);
            if (ret == AVERROR(EAGAIN))
            {
                frame = nullptr;
                ret = 0;
            }

        } while (ret == 0 && !bstop);

        std::cout << "exit vdec_thread" << std::endl;
        return 0;
    }
    );

    // 视频编码
    gff::genc venc;
    ret = venc.set_video_param("h264_qsv", 2000000, 1920, 1080, { 1, FPS }, { FPS,1 }, FPS, 0, AV_PIX_FMT_NV12);
    CHECKFFRET(ret);
    ret = venc.get_codectx(vcodectx);
    CHECKFFRET(ret);
    // 视频编码线程
    std::thread venc_thread([&]()
    {
        int ret = 0;
        decltype(gff::GetPacket()) packet = nullptr;
        decltype(gff::GetFrame()) frame = nullptr;
        do
        {
            frame = nullptr;
            {
                std::lock_guard<decltype(vframe_queue_mutex)> _lock(vframe_queue_mutex);
                if (vframe_queue.size() > 0)
                {
                    frame = vframe_queue.front();
                    vframe_queue.pop();
                }
            }

            if (frame != nullptr)
            {
                /*std::cout << "got a vframe, pts " <<
                    av_rescale_q(frame->pts, vtimebase, { 1,1 }) << std::endl;*/
                ret = venc.encode_push_frame(frame);
                CHECKFFRET(ret);
                do
                {
                    packet = gff::GetPacket();
                    ret = venc.encode_get_packet(packet);
                    packet->pts = frame->pts;
                    /*std::cout << "got a vpacket, pts " <<
                        av_rescale_q(packet->pts, vtimebase, { 1,1 }) << std::endl;*/
                    {
                        std::lock_guard<decltype(opacket_queue_mutex)> _lock(opacket_queue_mutex);
                        if (opacket_queue.size() > 100)
                        {
                            std::queue<std::shared_ptr<AVPacket>> empty;
                            opacket_queue.swap(empty);
                            std::cout << "too many opackets" << std::endl;
                        }
                        opacket_queue.push(packet);
                    }
                } while (ret == 0);
                if (ret == AVERROR(EAGAIN))
                {
                    ret = 0;
                }
            }       

        } while (ret == 0 && !bstop);

        std::cout << "exit venc_thread" << std::endl;
        return 0;
    }
    );

    // 视频封装
    gff::gmux mux;
    ret = mux.create_output("out.mp4");
    CHECKFFRET(ret);
    ret = mux.create_stream(vcodectx, ovindex);
    CHECKFFRET(ret);
    ret = mux.write_header();
    CHECKFFRET(ret);
    ret = mux.get_timebase(ovindex, ovtimebase);
    CHECKFFRET(ret);
    // 视频封装线程
    std::thread mux_thread([&]()
    {
        int ret = 0;
        decltype(gff::GetPacket()) packet = nullptr;
        decltype(gff::GetFrame()) frame = nullptr;
        int64_t lastpts = -1;
        do
        {
            packet = nullptr;
            {
                std::lock_guard<decltype(opacket_queue_mutex)> _lock(opacket_queue_mutex);
                if (opacket_queue.size() > 0)
                {
                    packet = opacket_queue.front();
                    opacket_queue.pop();
                }
            }
            
            if (packet != nullptr)
            {
                packet->pts = av_rescale_q(packet->pts, vtimebase, ovtimebase);
                if (packet->pts <= lastpts)
                {
                    packet->pts = lastpts + 1;
                    
                }
                packet->duration = 1;
                packet->dts = packet->pts;
                lastpts = packet->pts;
                
                /*std::cout << "write a vpacket, pts " <<
                    av_rescale_q(packet->pts, ovtimebase, { 1,1 }) << std::endl;*/
                ret = mux.write_packet(packet);
                CHECKFFRET(ret);
            }
        } while (ret == 0 && !bstop);

        std::cout << "exit mux_thread" << std::endl;
        return 0;
    }
    );
    
    char buf[10] = { 0 };
    while (std::cin.getline(buf, sizeof(buf)) && buf[0] != 'q')
    {
    }

    bstop = true;
    if (demux_desktop_thread.joinable())
    {
        demux_desktop_thread.join();
    }
    if (vdec_thread.joinable())
    {
        vdec_thread.join();
    }
    if (venc_thread.joinable())
    {
        venc_thread.join();
    }
    if (mux_thread.joinable())
    {
        mux_thread.join();
    }

    std::cin.get();

    return 0;
}