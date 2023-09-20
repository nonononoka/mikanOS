#pragma once

#include <stdint.h>

enum PixelFormat {
    kPixelRGBResv8BitPerColor,
    kPixelBGRResv8BitPerColor,
};

struct FrameBufferConfig {
  uint8_t* frame_buffer; //pointer 
  uint32_t pixels_per_scan_line; //pixel in lateral direction
  uint32_t horizontal_resolution; 
  uint32_t vertical_resolution;
  enum PixelFormat pixel_format; //data type of pixel
};
