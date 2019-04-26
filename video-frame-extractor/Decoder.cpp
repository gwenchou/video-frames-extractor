//
//  Decoder.cpp
//  Lab
//
//  Created by Jay Chou on 2019/3/6.
//  Copyright © 2019 Jay Chou. All rights reserved.
//
#include <iostream>

#include "Decoder.hpp"
using namespace std;

Decoder::Decoder(string file_path)
: file_path_(file_path)
, pFormatCtx_(avformat_alloc_context())
, pVideoCodecCtx_(nullptr)
, pAudioCodecCtx_(nullptr)
, pVideoCodec_(nullptr)
, pAudioCodec_(nullptr)
, pVideoFrameYUV_(av_frame_alloc())
, pVideoFrameRGB_(av_frame_alloc())
, pVideoStreamIndex_(-1)
, pAudioStreamIndex_(-1)
, pSwsCtx_(nullptr)
{
    avdevice_register_all();

    // Open video file
    if (avformat_open_input(&pFormatCtx_, file_path_.c_str(), nullptr, nullptr) != 0)
    {
        throw runtime_error("Couldn't open file, file name: " + file_path_);
    }
    cout << "Decoder open file successful, filename:" << file_path_ << "\n";
    
    // Retrieve stream information
    if (avformat_find_stream_info(pFormatCtx_, nullptr) < 0)
    {
        throw runtime_error("Couldn'ts find stream information");
    }
    
    AVDictionary *avoptDict = nullptr;
    av_dict_set(&avoptDict, "threads", "6", 0);
    
    // Get the video stream and audio stream
    if((pVideoStreamIndex_ = av_find_best_stream(pFormatCtx_, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0)) > -1)
    {
        pVideoStream_ = pFormatCtx_->streams[pVideoStreamIndex_];
        if (pVideoStream_ == nullptr)
        {
            cout << "Didn't find a video stream\n";
        }
        
        // Find the decoder for the audio stream
        if ((pVideoCodec_ = avcodec_find_decoder(pVideoStream_->codecpar->codec_id)) == nullptr)
        {
            avcodec_free_context(&pVideoCodecCtx_);
            cout << "Unsupported video codec\n";
        }
        
        pVideoCodecCtx_ = avcodec_alloc_context3(pVideoCodec_);
        avcodec_parameters_to_context(pVideoCodecCtx_, pVideoStream_->codecpar);
        
        // Open video codec
        if (avcodec_open2(pVideoCodecCtx_, pVideoCodec_, &avoptDict) < 0)
        {
            avcodec_free_context(&pVideoCodecCtx_);
            cout << "Could not open video codec\n";
        }
        
        pSwsCtx_ = sws_getContext(pVideoCodecCtx_->width, pVideoCodecCtx_->height, pVideoCodecCtx_->pix_fmt, pVideoCodecCtx_->width, pVideoCodecCtx_->height, AV_PIX_FMT_RGB24, SWS_BICUBIC, nullptr, nullptr, nullptr);
        
        float frame_rate = av_q2d(pVideoStream_->r_frame_rate);
        videoFrameCount_ = pVideoStream_->nb_frames;
        videoHeight_ = pVideoCodecCtx_->height;
        videoWidth_ = pVideoCodecCtx_->width;
        
        cout << "Got video stream, width: " << videoWidth_ << "px, height: " << videoHeight_ << "px, frame rate: " << frame_rate << " fps, duration: " << pFormatCtx_->duration / 1000000 << " sec, frame count: " << videoFrameCount_ << " frames\n";
    }
    
    if((pAudioStreamIndex_ = av_find_best_stream(pFormatCtx_, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0)) > -1)
    {
        pAudioStream_ = pFormatCtx_->streams[pAudioStreamIndex_];
        if (pAudioStream_ == nullptr)
        {
            cout << "Didn't find a audio stream\n";
        }
        
        // Find the decoder for the audio stream
        if ((pAudioCodec_ = avcodec_find_decoder(pAudioStream_->codecpar->codec_id)) == nullptr)
        {
            avcodec_free_context(&pAudioCodecCtx_);
            cout << "Unsupported audio codec\n";
        }
        
        pAudioCodecCtx_ = avcodec_alloc_context3(pAudioCodec_);
        avcodec_parameters_to_context(pAudioCodecCtx_, pAudioStream_->codecpar);
        
        // Open audio codec
        if (avcodec_open2(pAudioCodecCtx_, pAudioCodec_, &avoptDict) < 0)
        {
            avcodec_free_context(&pAudioCodecCtx_);
            cout << "Could not open audio codec\n";
        }
        
        cout << "Got audio stream, sample rate: " << pAudioCodecCtx_->sample_rate << " Hz, channels: " << pAudioCodecCtx_->channels << "\n";
    }
    
    //Prepare RGB frame
    int buffer_size = av_image_get_buffer_size(AV_PIX_FMT_RGB24, pVideoCodecCtx_->width, pVideoCodecCtx_->height, 1);
    uint8_t *buffer = (uint8_t*)av_malloc(buffer_size * sizeof(uint8_t));
    av_image_fill_arrays(pVideoFrameRGB_->data, pVideoFrameRGB_->linesize, (uint8_t*)buffer, AV_PIX_FMT_BGR24, pVideoCodecCtx_->width, pVideoCodecCtx_->height, 1);
    
    av_dict_free(&avoptDict);
}

AVFrame* Decoder::take_one_video_frame()
{
    int ret;
    AVPacket packet;
    
    while (av_read_frame(pFormatCtx_, &packet) >= 0)
    {
        if (packet.data != nullptr)
        {
            if (packet.stream_index != pVideoStreamIndex_)
            {
                av_packet_unref(&packet);
                continue;
            }
        }
        
        if (packet.pos != -1 && packet.duration != 0)
        {
            ret = avcodec_send_packet(pVideoCodecCtx_, &packet);
        }
        av_packet_unref(&packet);
        
        if (ret < 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        }
        
        if (ret  >= 0) {
            ret = avcodec_receive_frame(pVideoCodecCtx_, pVideoFrameYUV_);
            if (ret >= 0)
            {
                break;
            }
        }
    }
    
    sws_scale(pSwsCtx_, (uint8_t const * const *)pVideoFrameYUV_->data,
              pVideoFrameYUV_->linesize, 0, pVideoCodecCtx_->height,
              pVideoFrameRGB_->data, pVideoFrameRGB_->linesize);
    
    return pVideoFrameRGB_;
}
