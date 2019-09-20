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

int main(int argc, char* argv[])
{
    int ret = 0;

    gff::gdemux demux_desktop;
    ret = demux_desktop.open("desktop", "gdigrab", { {"framerate", "30"} });
    CHECKFFRET(ret);

    std::vector<unsigned int> videovec, audiovec;
    ret = demux_desktop.get_steam_index(videovec, audiovec);
    CHECKFFRET(ret);
    int vindex = videovec.size() > 0 ? videovec.at(0) : -1;

    const AVCodecParameters* vpar = nullptr;
    AVRational vtimebase;
    ret = demux_desktop.get_stream_par(vindex, vpar, vtimebase);
    CHECKFFRET(ret);

    std::queue<std::shared_ptr<AVPacket>> packet_queue;
    decltype(gff::GetPacket()) packet = nullptr;
    do
    {
        if (packet != nullptr)
        {
            if (packet_queue.size() > 100)
            {
                decltype(packet_queue) empty;
                packet_queue.swap(empty);
            }
            packet_queue.push(packet);
            std::cout << "got a packet, index " << packet->stream_index << " pts " <<
                av_rescale_q(packet->pts, vtimebase, { 1,1 }) << std::endl;
        }

        packet = gff::GetPacket();
        if (packet == nullptr)
        {
            CHECKFFRET(AVERROR(ENOMEM));
        }

    } while (demux_desktop.readpacket(packet) == 0);

    return 0;
}