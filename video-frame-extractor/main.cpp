//
//  main.cpp
//  video-frame-extractor
//
//  Created by Jay Chou on 2019/3/17.
//  Copyright © 2019 Jay Chou. All rights reserved.
//

#include <iostream>
#include <jpeglib.h>
#include "Decoder.hpp"

static void save_frame_jpeg(FILE *file, uint8_t *pRGBBuffer, int width, int height);

int main(int argc, const char * argv[]) {
    string input_path = argv[1];
    string output_path = argv[2];
    
    Decoder *decoder = new Decoder(input_path);

    AVFrame *video_frame;
    char file_name[32];
    for (long long i = 0; i < decoder->videoFrameCount_; i++) {
        sprintf(file_name, "frame_%lld.jpg", i);
        string file_path = output_path + file_name;

        video_frame = decoder->take_one_video_frame();
        save_frame_jpeg(fopen(file_path.c_str(), "wb"), video_frame->data[0], decoder->videoWidth_, decoder->videoHeight_);
    }
    return 0;
}

static void save_frame_jpeg(FILE *file, uint8_t *pRGBBuffer, int width, int height)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    int row_stride;
    
    JSAMPROW row_pointer[1];
    
    cinfo.err = jpeg_std_error(&jerr);
    
    jpeg_create_compress(&cinfo);
    
    if(file == NULL)
        return;
    
    jpeg_stdio_dest(&cinfo, file);
    
    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 80, TRUE);
    
    jpeg_start_compress(&cinfo, TRUE);
    
    
    row_stride = cinfo.image_width * 3;
    
    // compress
    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = &(pRGBBuffer[cinfo.next_scanline * row_stride]);
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }
    
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    
    fclose(file);
};
