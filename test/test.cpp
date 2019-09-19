#include <iostream>
#include <fstream>
#include "../src/gutil.h"
#include "../src/gdemux.h"
#include "../src/gdec.h"
#include "../src/genc.h"
#include "../src/gmux.h"
#include "../src/gsws.h"
#include "../src/gswr.h"

int test_demux(const char* in)
{
    gff::gdemux demux;
    demux.open(in);
    auto packet = gff::GetPacket();
    while (demux.readpacket(packet) == 0)
    {
        std::cout << "pts : " << packet->pts << " " << packet->stream_index << std::endl;
    }
    demux.cleanup();

    return 0;
}

int test_dec(const char* in)
{
    gff::gdemux demux;
    demux.open(in);
    
    auto packet = gff::GetPacket();
    auto frame = gff::GetFrame();

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

    while (demux.readpacket(packet) == 0)
    {
        if (packet->stream_index == videovec.at(0))
        {
            if (vdec.decode(packet, frame) >= 0)
            {
                do
                {
                    std::cout << "pts : " << frame->pts << " " << packet->stream_index << std::endl;
                    static std::ofstream f("out.yuv", std::ios::binary | std::ios::trunc);
                    f.write(reinterpret_cast<const char*>(frame->data[0]), static_cast<int64_t>(frame->linesize[0]) * frame->height);
                    f.write(reinterpret_cast<const char*>(frame->data[1]), static_cast<int64_t>(frame->linesize[1]) * frame->height / 2);
                    f.write(reinterpret_cast<const char*>(frame->data[2]), static_cast<int64_t>(frame->linesize[2]) * frame->height / 2);
                } while (vdec.decode(nullptr, frame) >= 0);
            }
        }
        else if (packet->stream_index == audiovec.at(0))
        {
            if (adec.decode(packet, frame) >= 0)
            {
                do
                {
                    std::cout << "pts : " << frame->pts << " " << packet->stream_index << std::endl;
                    static std::ofstream f("out.pcm", std::ios::binary | std::ios::trunc);
                    auto size = av_get_bytes_per_sample(static_cast<AVSampleFormat>(frame->format));
                    for (int i = 0; i < frame->nb_samples; ++i)
                    {
                        for (int j = 0; j < frame->channels; ++j)
                        {
                            f.write(reinterpret_cast<const char*>(frame->data[j] + size * i), size);
                        }
                    }
                } while (adec.decode(nullptr, frame) >= 0);
            }
        }
    }
    demux.cleanup();

    return 0;
}

int test_enc_video(const char* in)
{
    std::ifstream yuv(in, std::ios::binary);
    char* buf = static_cast<char*>(malloc(4608000));
    if (buf == nullptr)
    {
        return 0;
    }

    gff::genc enc;
    enc.set_video_param("libx264", 400000, 640, 480, { 1,25 }, { 25,1 }, 5, 0, AV_PIX_FMT_YUV420P);

    auto packet = gff::GetPacket();
    auto frame = gff::GetFrame();
    frame->width = 640;
    frame->height = 480;
    frame->format = AV_PIX_FMT_YUV420P;
    av_frame_get_buffer(frame.get(), 1);

    while (!yuv.eof())
    {
        yuv.read(buf, 4608000);
        av_frame_make_writable(frame.get());
        memcpy(frame->data[0], buf, frame->linesize[0] * frame->height);
        memcpy(frame->data[1], buf + frame->linesize[0] * frame->height, frame->linesize[1] * frame->height / 2);
        memcpy(frame->data[2], buf + frame->linesize[0] * frame->height * 5 / 4, frame->linesize[2] * frame->height / 2);
        static int i = 0;
        frame->pts = i++;
        if (enc.encode_push_frame(frame) >= 0)
        {
            while (enc.encode_get_packet(packet) >= 0)
            {
                std::cout << "pts : " << packet->pts << std::endl;
            }
        }
    }

    enc.encode_push_frame(nullptr);
    while (enc.encode_get_packet(packet) >= 0)
    {
        std::cout << "pts : " << packet->pts << std::endl;
    }
    enc.cleanup();
    free(buf);
    buf = nullptr;

    return 0;
}

int test_enc_audio(const char* in)
{
    std::ifstream pcm("out.pcm", std::ios::binary);
    char buf[10240] = { 0 };
    gff::genc enc;
    int framesize = 0;

    enc.set_audio_param("libmp3lame", 64000, 44100, AV_CH_LAYOUT_STEREO, 2, AV_SAMPLE_FMT_FLTP, framesize);

    auto packet = gff::GetPacket();
    auto frame = gff::GetFrame();
    frame->nb_samples = framesize;
    frame->format = AV_SAMPLE_FMT_FLTP;
    frame->channel_layout = AV_CH_LAYOUT_STEREO;
    auto size = av_get_bytes_per_sample(static_cast<AVSampleFormat>(frame->format));
    av_frame_get_buffer(frame.get(), 0);

    while (!pcm.eof())
    {
        auto len = static_cast<std::streamsize>(framesize) * size * av_get_channel_layout_nb_channels(frame->channel_layout);
        pcm.read(buf, static_cast<std::streamsize>(framesize) * size * av_get_channel_layout_nb_channels(frame->channel_layout));
        av_frame_make_writable(frame.get());

        for (int i = 0; i < frame->nb_samples; ++i)
        {
            memcpy_s(frame->data[0] + size * i, size, buf + size * (2 * i), size);
            memcpy_s(frame->data[1] + size * i, size, buf + size * (2 * i + 1), size);
        }
        static int i = 0;
        frame->pts = i++;
        if (enc.encode_push_frame(frame) >= 0)
        {
            while (enc.encode_get_packet(packet) >= 0)
            {
                std::cout << "pts : " << packet->pts << std::endl;
            }
        }
    }

    enc.encode_push_frame(nullptr);
    while (enc.encode_get_packet(packet) >= 0)
    {
        std::cout << "pts : " << packet->pts << std::endl;
    }
    enc.cleanup();

    return 0;
}

int test_mux(const char* out)
{
    std::ifstream yuv("out.yuv", std::ios::binary);
    char* buf = static_cast<char*>(malloc(4608000));
    if (buf == nullptr)
    {
        return 0;
    }

    gff::genc enc;
    enc.set_video_param("libx264", 8000000, 640, 480, { 1,15 }, { 15,1 }, 5, 0, AV_PIX_FMT_YUV420P);

    gff::gmux mux;
    mux.create_output(out);
    const AVCodecContext* codectx = nullptr;
    enc.get_codectx(codectx);
    int vindex = -1;
    mux.create_stream(codectx, vindex);
    mux.write_header();
    AVRational timebase = { 0 };
    mux.get_timebase(vindex, timebase);

    auto packet = gff::GetPacket();
    auto frame = gff::GetFrame();
    frame->width = 640;
    frame->height = 480;
    frame->format = AV_PIX_FMT_YUV420P;
    av_frame_get_buffer(frame.get(), 1);

    while (!yuv.eof())
    {
        yuv.read(buf, 4608000);
        av_frame_make_writable(frame.get());
        memcpy(frame->data[0], buf, frame->linesize[0] * frame->height);
        memcpy(frame->data[1], buf + frame->linesize[0] * frame->height, frame->linesize[1] * frame->height / 2);
        memcpy(frame->data[2], buf + frame->linesize[0] * frame->height * 5 / 4, frame->linesize[2] * frame->height / 2);
        static int i = 0;
        frame->pts = i++;
        if (enc.encode_push_frame(frame) >= 0)
        {
            while (enc.encode_get_packet(packet) >= 0)
            {
                packet->pts = av_rescale_q(packet->pts, { 1, 15 }, timebase);
                std::cout << "pts : " << packet->pts << std::endl;
                mux.write_packet(packet);
            }
        }
    }

    enc.encode_push_frame(nullptr);
    while (enc.encode_get_packet(packet) >= 0)
    {
        packet->pts = av_rescale_q(packet->pts, { 1, 15 }, timebase);
        std::cout << "pts : " << packet->pts << std::endl;
        mux.write_packet(packet);
    }
    mux.cleanup();
    enc.cleanup();
    free(buf);
    buf = nullptr;

    return 0;
}

int test_sws(const char* in)
{
    std::ifstream yuv(in, std::ios::binary);
    std::ofstream argb("out.argb", std::ios::binary);
    char* buf = static_cast<char*>(malloc(4608000));
    if (buf == nullptr)
    {
        return 0;
    }

    auto packet = gff::GetPacket();
    auto frame = gff::GetFrame();
    auto frame2 = gff::GetFrame();
    frame->width = 640;
    frame->height = 480;
    frame->format = AV_PIX_FMT_YUV420P;
    av_frame_get_buffer(frame.get(), 1);

    frame2->width = 1920;
    frame2->height = 1080;
    frame2->format = AV_PIX_FMT_RGB24;
    av_frame_get_buffer(frame2.get(), 1);

    gff::gsws sws;
    sws.create_sws(static_cast<AVPixelFormat>(frame->format), frame->width, frame->height,
        static_cast<AVPixelFormat>(frame2->format), frame2->width, frame2->height);

    while (!yuv.eof())
    {
        yuv.read(buf, 4608000);
        av_frame_make_writable(frame.get());
        memcpy(frame->data[0], buf, frame->linesize[0] * frame->height);
        memcpy(frame->data[1], buf + frame->linesize[0] * frame->height, frame->linesize[1] * frame->height / 2);
        memcpy(frame->data[2], buf + frame->linesize[0] * frame->height * 5 / 4, frame->linesize[2] * frame->height / 2);
        
        av_frame_make_writable(frame2.get());
        int h = sws.scale(frame->data, frame->linesize, 0, frame->height, frame2->data, frame2->linesize);
        argb.write(reinterpret_cast<char*>(frame2->data[0]), static_cast<std::streamsize>(frame2->linesize[0])*h);
    }

    sws.cleanup();
    free(buf);
    buf = nullptr;

    return 0;
}

int test_swr(const char* in)
{
    std::ifstream pcm("out.pcm", std::ios::binary);
    std::ofstream pcm2("out2.pcm", std::ios::binary);
    char* buf = static_cast<char*>(malloc(44100*2*4));

    auto packet = gff::GetPacket();
    auto frame = gff::GetFrame();
    auto frame2 = gff::GetFrame();

    frame->nb_samples = 44100;
    frame->format = AV_SAMPLE_FMT_FLTP;
    frame->channel_layout = AV_CH_LAYOUT_STEREO;
    auto size = av_get_bytes_per_sample(static_cast<AVSampleFormat>(frame->format));
    av_frame_get_buffer(frame.get(), 0);

    frame2->nb_samples = 48000;
    frame2->format = AV_SAMPLE_FMT_S32P;
    frame2->channel_layout = AV_CH_LAYOUT_STEREO;
    auto size2 = av_get_bytes_per_sample(static_cast<AVSampleFormat>(frame2->format));
    av_frame_get_buffer(frame2.get(), 0);

    gff::gswr swr;
    swr.create_swr(frame->channel_layout, frame->nb_samples, static_cast<AVSampleFormat>(frame->format),
        frame2->channel_layout, frame2->nb_samples, static_cast<AVSampleFormat>(frame2->format));

    // 获取样本格式对应的每个样本大小(Byte)
    int persize = av_get_bytes_per_sample(static_cast<AVSampleFormat>(frame2->format));
    // 获取布局对应的通道数
    int channel = av_get_channel_layout_nb_channels(frame2->channel_layout);

    while (!pcm.eof())
    {
        auto len = static_cast<std::streamsize>(frame->nb_samples) * size * av_get_channel_layout_nb_channels(frame->channel_layout);
        pcm.read(buf, static_cast<std::streamsize>(frame->nb_samples) * size * av_get_channel_layout_nb_channels(frame->channel_layout));
        av_frame_make_writable(frame.get());

        for (int i = 0; i < frame->nb_samples; ++i)
        {
            memcpy_s(frame->data[0] + size * i, size, buf + size * (2 * i), size);
            memcpy_s(frame->data[1] + size * i, size, buf + size * (2 * i + 1), size);
        }
        
        auto size = swr.convert(frame2->data, frame2->linesize[0], (const uint8_t * *)(frame->data), frame->nb_samples);
        // 拷贝音频数据
        for (int i = 0; i < size; ++i) // 每个样本
        {
            for (int j = 0; j < channel; ++j) // 每个通道
            {
                pcm2.write(reinterpret_cast<const char*>(frame2->data[j] + persize * i), persize);
            }
        }
    }

    free(buf);
    return 0;
}

int main(int argc, const char* argv[])
{
    std::cout << "hello g-ffmpeg!" << std::endl;
    //av_log_set_level(AV_LOG_TRACE);

    test_demux("gx.mkv");// gx.mkv在https://github.com/gongluck/RandB/blob/master/media/gx.mkv
    test_dec("gx.mkv");// gx.mkv在https://github.com/gongluck/RandB/blob/master/media/gx.mkv
    test_enc_video("out.yuv");// out.yuv这个文件太大了，没有上传github，可以用解码的例子生成
    test_enc_audio("out.pcm");// out.pcm这个文件太大了，没有上传github，可以用解码的例子生成
    test_mux("out.mp4");//out.yuv
    test_sws("out.yuv");//out.yuv
    test_swr("out.pcm");//out.pcm

    std::cin.get();
    return 0;
}