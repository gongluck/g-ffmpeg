/*******************************************************************
*  Copyright(c) 2019
*  All rights reserved.
*
*  文件名称:    gdec.cpp
*  简要描述:    解码
*
*  作者:  gongluck
*  说明:
*
*******************************************************************/

#include "gdec.h"
#include "gutil.h"

namespace gff
{
	gdec::~gdec()
	{
		cleanup();
	}

	int gdec::cleanup()
	{
		LOCK();

		avcodec_free_context(&codectx_);
		getstatus() = STOP;

		return 0;
	}

	int gdec::copy_param(const AVCodecParameters* par, AVHWDeviceType hwtype /*= AV_HWDEVICE_TYPE_NONE*/)
	{
		LOCK();
		CHECKSTOP();
		int ret = 0;

		cleanup();

		auto codec = avcodec_find_decoder(par->codec_id);
		if (codec == nullptr)
		{
			CHECKFFRET(AVERROR(EINVAL));
		}
		codectx_ = avcodec_alloc_context3(codec);
		if (codectx_ == nullptr)
		{
			CHECKFFRET(AVERROR(ENOMEM));
		}

		ret = avcodec_parameters_to_context(codectx_, par);
		CHECKFFRET(ret);

		if (hwtype != AV_HWDEVICE_TYPE_NONE)
		{
			// 查询硬解码支持
			for (int i = 0;; i++)
			{
				const AVCodecHWConfig* config = avcodec_get_hw_config(codec, i);
				if (config == nullptr)
				{
					ret = AVERROR(EINVAL);
					CHECKFFRET(ret);
				}
				if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
					config->device_type == hwtype)
				{
					// 硬解上下文
					AVBufferRef* hwbufref = nullptr;
					ret = av_hwdevice_ctx_create(&hwbufref, hwtype, nullptr, nullptr, 0);
					CHECKFFRET(ret);

					codectx_->hw_device_ctx = av_buffer_ref(hwbufref);
					if (codectx_->hw_device_ctx == nullptr)
					{
						ret = AVERROR_BUG;
						CHECKFFRET(ret);
					}
					av_buffer_unref(&hwbufref);

					break;
				}
			}
		}

		ret = avcodec_open2(codectx_, codec, nullptr);
		CHECKFFRET(ret);

		getstatus() = WORKING;

		return 0;
	}

	int gdec::decode(std::shared_ptr<AVPacket> packet, std::shared_ptr<AVFrame> frame)
	{
		LOCK();
		CHECKNOTSTOP();

		if (codectx_ == nullptr)
		{
			CHECKFFRET(AVERROR(EINVAL));
		}

		if (packet != nullptr)
		{
			// 发送将要解码的数据
			int ret = avcodec_send_packet(codectx_, packet.get());
			CHECKFFRET(ret);
		}

		// 接收解码数据
		return avcodec_receive_frame(codectx_, frame.get());
	}
}//gff