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
#include "../src/gutil.h"
#include "../src/gdemux.h"

int main(int argc, char* argv[])
{
    int ret = 0;

    gff::gdemux demux_desktop;
    ret = demux_desktop.open("desktop", "gdigrab", { {"framerate", "30"} });
    CHECKFFRET(ret);
    AVPacket desktop_pkt;
    av_init_packet(&desktop_pkt);
    while (demux_desktop.readpacket(desktop_pkt) == 0)
    {
        std::cout << "got a packt " << std::endl;
    }

    return 0;
}