#include <cstdint>
#include <cstddef>

#include "frame_buffer_config.hpp"

struct PixelColor{
    uint8_t r,g,b;
};

int WritePixel(const FrameBufferConfig& config,
               int x, int y, const PixelColor& c){
    const int pixel_position = config.pixels_per_scan_line * y + x;
    if (config.pixel_format == kPixelRGBResv8BitPerColor){
        uint8_t * p = &config.frame_buffer[4 * pixel_position];
        p[0] = c.b;
        p[1] = c.g;
        p[2] = c.r;
    }
    else if (config.pixel_format == kPixelBGRResv8BitPerColor){
        uint8_t *p = &config.frame_buffer[4 * pixel_position]; //1 pixel is 4 bytes. r(8bits) g b reservation region
        p[0] = c.b;
        p[1] = c.g;
        p[2] = c.r;
    }
    else{
        return -1;
    }
    return 0;  
}
extern "C" void KernelMain(const FrameBufferConfig& frame_buffer_config){ //extern "C" means that we can define function in C language.
    for (int x = 0;x < frame_buffer_config.horizontal_resolution; ++x){
        for (int y = 0;y < frame_buffer_config.vertical_resolution;++y){
            WritePixel(frame_buffer_config,x,y,{255,255,255});
        }
    }
    for (int x = 0; x < 200; ++x) {
        for (int y = 0; y < 100; ++y) {
            WritePixel(frame_buffer_config, 100 + x, 100 + y, {0, 255, 0});
        }
    }
    while(1) __asm__("hlt"); //inline assemblar inorder to embed assembly language into C language.
}
//"hlt means we want to stop CPU"