//
//  Decoder.hpp
//  Lab
//
//  Created by Jay Chou on 2019/3/6.
//  Copyright © 2019 Jay Chou. All rights reserved.
//

#ifndef Decoder_hpp
#define Decoder_hpp

#include <stdio.h>
#include <string>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/rational.h>
#include <libavutil/imgutils.h>
#include <libavdevice/avdevice.h>
}

using namespace std;

class Decoder
{
public:
    Decoder(string file_path);
    ~Decoder();
    
    AVFrame* take_one_video_frame();
    
    long long videoFrameCount_;
    int videoHeight_;
    int videoWidth_;
private:
    string file_path_;
    
    AVFormatContext *pFormatCtx_;
    AVStream        *pVideoStream_;
    AVStream        *pAudioStream_;
    AVCodecContext  *pVideoCodecCtx_;
    AVCodecContext  *pAudioCodecCtx_;
    AVCodec         *pVideoCodec_;
    AVCodec         *pAudioCodec_;
    AVFrame         *pVideoFrameYUV_;
    AVFrame         *pVideoFrameRGB_;
    
    struct SwsContext *pSwsCtx_;
    
    int pVideoStreamIndex_;
    int pAudioStreamIndex_;
};

#endif /* Decoder_hpp */
