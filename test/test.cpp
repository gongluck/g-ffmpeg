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
    const AVCodecParameters* par = nullptr;
    AVRational timebase;
    demux.get_stream_par(0, par, timebase);
    auto packet = gff::GetPacket();
    while (demux.readpacket(packet) == 0)
    {
        std::cout << "pts : " << av_rescale_q(packet->pts, timebase, { 1,1 }) << " " << packet->stream_index << std::endl;
        packet = gff::GetPacket();
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
    AVRational vtimebase, atimebase;
    const AVCodecParameters* apar = nullptr;
    demux.get_stream_par(videovec.at(0), vpar, vtimebase);
    demux.get_stream_par(audiovec.at(0), apar, atimebase);

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
                    std::cout << "pts : " << av_rescale_q(frame->pts, vtimebase, { 1,1 }) << " " << packet->stream_index << std::endl;
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
                    std::cout << "pts : " << av_rescale_q(frame->pts, atimebase, { 1,1 }) << " " << packet->stream_index << std::endl;
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
        packet = gff::GetPacket();
        frame = gff::GetFrame();
    }

    vdec.cleanup();
    adec.cleanup();
    demux.cleanup();

    return 0;
}

int test_enc_video(const char* in)
{
    const int width = 640;
    const int height = 480;
    const int sizelen = width * height * 3 / 2;
    std::ifstream yuv(in, std::ios::binary);
    char* buf = static_cast<char*>(malloc(sizelen));
    if (buf == nullptr)
    {
        return 0;
    }

    gff::genc enc;
    enc.set_video_param("libx264", 400000, width, height, { 1,25 }, { 25,1 }, 5, 0, AV_PIX_FMT_YUV420P);
    const AVCodecContext* codectx = nullptr;
    enc.get_codectx(codectx);

    while (!yuv.eof())
    {
        auto packet = gff::GetPacket();
        auto frame = gff::GetFrame();
        gff::GetFrameBuf(frame, width, height, AV_PIX_FMT_YUV420P, 1);

        yuv.read(buf, sizelen);
        gff::frame_make_writable(frame);
        memcpy(frame->data[0], buf, frame->linesize[0] * frame->height);
        memcpy(frame->data[1], buf + frame->linesize[0] * frame->height, frame->linesize[1] * frame->height / 2);
        memcpy(frame->data[2], buf + frame->linesize[0] * frame->height * 5 / 4, frame->linesize[2] * frame->height / 2);
        static int i = 0;
        frame->pts = i++;
        if (enc.encode_push_frame(frame) == 0)
        {
            while (enc.encode_get_packet(packet) == 0)
            {
                std::cout << "pts : " << av_rescale_q(packet->pts, codectx->time_base, { 1,1 }) << std::endl;
            }
        }
    }

    enc.encode_push_frame(nullptr);
    auto packet = gff::GetPacket();
    while (enc.encode_get_packet(packet) == 0)
    {
        std::cout << "pts : " << av_rescale_q(packet->pts, codectx->time_base, { 1,1 }) << std::endl;
    }
    enc.cleanup();
    free(buf);
    buf = nullptr;

    return 0;
}

int test_enc_audio(const char* in)
{
    const int bufsize = 10240;
    std::ifstream pcm("out.pcm", std::ios::binary);
    char buf[bufsize] = { 0 };
    gff::genc enc;
    int framesize = 0;

    enc.set_audio_param("libmp3lame", 64000, 48000, AV_CH_LAYOUT_STEREO, 2, AV_SAMPLE_FMT_FLTP, framesize);

    auto packet = gff::GetPacket();
    auto frame = gff::GetFrame();
    gff::GetFrameBuf(frame, framesize, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_FLTP, 1);
    auto size = av_get_bytes_per_sample(static_cast<AVSampleFormat>(frame->format));

    const AVCodecContext* codectx = nullptr;
    enc.get_codectx(codectx);

    while (!pcm.eof())
    {
        auto len = static_cast<std::streamsize>(framesize) * size * av_get_channel_layout_nb_channels(frame->channel_layout);
        pcm.read(buf, static_cast<std::streamsize>(framesize) * size * av_get_channel_layout_nb_channels(frame->channel_layout));
        gff::frame_make_writable(frame);

        for (int i = 0; i < frame->nb_samples; ++i)
        {
            memcpy_s(frame->data[0] + size * i, size, buf + size * (2 * i), size);
            memcpy_s(frame->data[1] + size * i, size, buf + size * (2 * i + 1), size);
        }
        static int i = 0;
        frame->pts = av_rescale_q(static_cast<int64_t>(frame->nb_samples) * i++, { 1, 48000 }, codectx->time_base);
        if (enc.encode_push_frame(frame) >= 0)
        {
            while (enc.encode_get_packet(packet) >= 0)
            {
                std::cout << "pts : " << av_rescale_q(packet->pts, codectx->time_base, { 1,1 }) << std::endl;
            }
        }
    }

    enc.encode_push_frame(nullptr);
    while (enc.encode_get_packet(packet) >= 0)
    {
        std::cout << "pts : " << av_rescale_q(packet->pts, codectx->time_base, { 1,1 }) << std::endl;
    }
    enc.cleanup();

    return 0;
}

int test_mux(const char* out)
{
    std::ifstream nv12("out.nv12", std::ios::binary);
    const int width = 640;
    const int height = 480;
    const int size = width * height * 3 / 2;
    AVRational ivtimebase = { 1, 24 };
    char* buf = static_cast<char*>(malloc(size));
    if (buf == nullptr)
    {
        return 0;
    }

    gff::genc enc;
    enc.set_video_param("h264_qsv", 2000000, width, height, ivtimebase, { 24,1 }, 5, 0, AV_PIX_FMT_NV12);

    gff::gmux mux;
    mux.create_output(out);
    const AVCodecContext* codectx = nullptr;
    enc.get_codectx(codectx);
    int vindex = -1;
    mux.create_stream(codectx, vindex);
    mux.write_header();
    AVRational ovtimebase;
    mux.get_timebase(vindex, ovtimebase);

    int i = 0;
    while (!nv12.eof())
    {
        auto packet = gff::GetPacket();
        auto frame = gff::GetFrame();
        gff::GetFrameBuf(frame, width, height, AV_PIX_FMT_NV12, 1);

        nv12.read(buf, size);
        av_frame_make_writable(frame.get());
        memcpy(frame->data[0], buf, frame->linesize[0] * frame->height);
        memcpy(frame->data[1], buf + frame->linesize[0] * frame->height, frame->linesize[1] * frame->height / 2);
        
        frame->pts = i++;
        if (enc.encode_push_frame(frame) == 0)
        {
            while (enc.encode_get_packet(packet) == 0)
            {
                packet->pts = av_rescale_q(packet->pts, ivtimebase, ovtimebase);
                packet->dts = packet->pts;
                packet->duration = 1;
                std::cout << "pts : " << av_rescale_q(packet->pts, ovtimebase, { 1,1 }) << std::endl;
                mux.write_packet(packet);
                packet = gff::GetPacket();
            }
        }
    }

    enc.encode_push_frame(nullptr);
    auto packet = gff::GetPacket();
    while (enc.encode_get_packet(packet) == 0)
    {
        packet->pts = av_rescale_q(packet->pts, ivtimebase, ovtimebase);
        packet->dts = packet->pts;
        packet->duration = 1;
        std::cout << "pts : " << av_rescale_q(packet->pts, ovtimebase, { 1,1 }) << std::endl;
        mux.write_packet(packet);
        packet = gff::GetPacket();
    }
    mux.cleanup();
    enc.cleanup();
    free(buf);
    buf = nullptr;

    return 0;
}

int test_sws(const char* in)
{
    const int width = 640;
    const int height = 480;
    const int sizelen = width * height * 3 / 2;
    std::ifstream yuv(in, std::ios::binary);
    std::ofstream nv12("out.nv12", std::ios::binary);
    char* buf = static_cast<char*>(malloc(sizelen));
    if (buf == nullptr)
    {
        return 0;
    }

    auto frame = gff::GetFrame();
    auto frame2 = gff::GetFrame();
    gff::GetFrameBuf(frame, width, height, AV_PIX_FMT_YUV420P, 1);
    gff::GetFrameBuf(frame2, width, height, AV_PIX_FMT_NV12, 1);

    gff::gsws sws;
    sws.create_sws(static_cast<AVPixelFormat>(frame->format), frame->width, frame->height,
        static_cast<AVPixelFormat>(frame2->format), frame2->width, frame2->height);

    while (!yuv.eof())
    {
        yuv.read(buf, sizelen);
        gff::frame_make_writable(frame);
        memcpy(frame->data[0], buf, frame->linesize[0] * frame->height);
        memcpy(frame->data[1], buf + frame->linesize[0] * frame->height, frame->linesize[1] * frame->height / 2);
        memcpy(frame->data[2], buf + frame->linesize[0] * frame->height * 5 / 4, frame->linesize[2] * frame->height / 2);

        gff::frame_make_writable(frame2);
        int h = sws.scale(frame->data, frame->linesize, 0, frame->height, frame2->data, frame2->linesize);
        nv12.write(reinterpret_cast<const char*>(frame2->data[0]), static_cast<int64_t>(frame2->linesize[0]) * h);
        nv12.write(reinterpret_cast<const char*>(frame2->data[1]), static_cast<int64_t>(frame2->linesize[1]) * h / 2);

        auto frame = gff::GetFrame();
        auto frame2 = gff::GetFrame();
        gff::GetFrameBuf(frame, width, height, AV_PIX_FMT_YUV420P, 1);
        gff::GetFrameBuf(frame2, width, height, AV_PIX_FMT_NV12, 1);
    }

    sws.cleanup();
    free(buf);
    buf = nullptr;

    std::cout << "sws end." << std::endl;

    return 0;
}

int test_swr(const char* in)
{
    std::ifstream pcm("out.pcm", std::ios::binary);
    std::ofstream pcm2("out2.pcm", std::ios::binary);
    const int bufsize = 48000 * 2 * 4;
    char* buf = static_cast<char*>(malloc(bufsize));

    auto packet = gff::GetPacket();

    auto frame = gff::GetFrame();
    gff::GetFrameBuf(frame, 48000, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_FLTP, 1);
    auto size = av_get_bytes_per_sample(static_cast<AVSampleFormat>(frame->format));
    gff::frame_make_writable(frame);

    auto frame2 = gff::GetFrame();
    gff::GetFrameBuf(frame2, 44100, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S32P, 1);
    auto size2 = av_get_bytes_per_sample(static_cast<AVSampleFormat>(frame2->format));
    gff::frame_make_writable(frame2);

    gff::gswr swr;
    swr.create_swr(frame->channel_layout, frame->nb_samples, static_cast<AVSampleFormat>(frame->format),
        frame2->channel_layout, frame2->nb_samples, static_cast<AVSampleFormat>(frame2->format));

    // 获取样本格式对应的每个样本大小(Byte)
    int persize = av_get_bytes_per_sample(static_cast<AVSampleFormat>(frame2->format));
    // 获取布局对应的通道数
    int channel = av_get_channel_layout_nb_channels(frame2->channel_layout);

    auto len = static_cast<std::streamsize>(frame->nb_samples) * size * channel;

    while (!pcm.eof())
    {
        pcm.read(buf, len);
        gff::frame_make_writable(frame);

        for (int i = 0; i < frame->nb_samples; ++i)
        {
            memcpy_s(frame->data[0] + size * i, size, buf + size * (2 * i), size);
            memcpy_s(frame->data[1] + size * i, size, buf + size * (2 * i + 1), size);
        }
        
        gff::frame_make_writable(frame2);
        auto swssize = swr.convert(frame2->data, frame2->linesize[0], (const uint8_t * *)(frame->data), frame->nb_samples);
        // 拷贝音频数据
        for (int i = 0; i < swssize; ++i) // 每个样本
        {
            for (int j = 0; j < channel; ++j) // 每个通道
            {
                pcm2.write(reinterpret_cast<const char*>(frame2->data[j] + persize * i), persize);
            }
        }
    }

    swr.cleanup();
    free(buf);
    std::cout << "swr end." << std::endl;

    return 0;
}

int main(int argc, const char* argv[])
{
    std::cout << "hello g-ffmpeg!" << std::endl;
    //av_log_set_level(AV_LOG_TRACE);

    //test_demux("gx.mkv");//gx.mkv在https://github.com/gongluck/RandB/blob/master/media/gx.mkv
    //test_dec("gx.mkv");//gx.mkv在https://github.com/gongluck/RandB/blob/master/media/gx.mkv
    //test_enc_video("out.yuv");//out.yuv这个文件太大了，没有上传github，可以用解码的例子生成
    //test_enc_audio("out.pcm");//out.pcm这个文件太大了，没有上传github，可以用解码的例子生成
    //test_sws("out.yuv");//out.yuv这个文件太大了，没有上传github，可以用解码的例子生成
    //test_swr("out.pcm");//out.pcm这个文件太大了，没有上传github，可以用解码的例子生成
    test_mux("out.mp4");//out.nv12这个文件太大了，没有上传github，可以用解码的例子生成

    std::cin.get();
    return 0;
}