#include <iostream>
#include "../src/gdemux.h"
#include "../src/gdec.h"

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

int test_dec(const char* in)
{
    gff::gdemux demux;
    demux.open(in);
    AVPacket packet;
    av_init_packet(&packet);

    std::vector<unsigned int> videovec, audiovec;
    demux.get_steam_index(videovec, audiovec);
    const AVCodecParameters* vpar = nullptr;
    const AVCodecParameters* apar = nullptr;
    demux.get_stream_par(videovec.at(0), vpar);
    demux.get_stream_par(audiovec.at(0), apar);

    gff::gdec vdec;
    vdec.copy_param(vpar);
    gff::gdec adec;
    adec.copy_param(apar);

    AVFrame frame = { 0 };
    while (demux.readpacket(packet) == 0)
    {
        //std::cout << "pts : " << packet.pts << " " << packet.stream_index << std::endl;
        if (packet.stream_index == videovec.at(0))
        {
            if (vdec.decode(&packet, frame) >= 0)
            {
                do
                {
                    std::cout << "pts : " << frame.pts << " " << packet.stream_index << std::endl;
                } while (vdec.decode(nullptr, frame) >= 0);
            }
        }
        else if (packet.stream_index == audiovec.at(0))
        {
            if (adec.decode(&packet, frame) >= 0)
            {
                do
                {
                    std::cout << "pts : " << frame.pts << " " << packet.stream_index << std::endl;
                } while (adec.decode(nullptr, frame) >= 0);
            }
        }
        // 不再引用指向的缓冲区
        av_packet_unref(&packet);
    }
    demux.close();

    return 0;
}

int main(int argc, const char* argv[])
{
    std::cout << "hello g-ffmpeg!" << std::endl;

    //test_demux("gx.mkv");
    test_dec("gx.mkv");

    std::cin.get();
    return 0;
}