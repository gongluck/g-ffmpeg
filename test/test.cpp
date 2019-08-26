#include <iostream>
#include "../src/gdemux.h"

int test_demux(const char* in)
{
    gff::gdemux demux;
    demux.open(in);
    AVPacket packet;
    av_init_packet(&packet);
    while (demux.readpacket(packet) == 0)
    {
        std::cout << "pts : " << packet.pts << " " << packet.stream_index << std::endl;
        // 不再引用指向的缓冲区
        av_packet_unref(&packet);
    }
    demux.close();

    return 0;
}

int main(int argc, const char* argv[])
{
    std::cout << "hello g-ffmpeg!" << std::endl;

    test_demux("gx.mkv");

    std::cin.get();
    return 0;
}