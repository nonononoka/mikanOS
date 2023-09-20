#include <cstdint>
#include <cstddef>

#include "frame_buffer_config.hpp"

struct PixelColor{
    uint8_t r,g,b;
};

class PixelWriter{
    public:
     PixelWriter(const FrameBufferConfig& config) : config_{config}{ //config_ is initialled as config

     } //constructor there is no description of type of return value
     virtual ~PixelWriter() = default; //destructor (maybe default means we will use default destructor function, but this is virtual)
     virtual void Write(int x, int y,const PixelColor& c) = 0; //pure virtual function(= 0 means pure)
     //pure virtual function is one that return type and name and argument is decided.

    protected:
     uint8_t* PixelAt(int x , int y){
        return config_.frame_buffer + 4 * (config_.pixels_per_scan_line*y + x);
    }

    private:
     const FrameBufferConfig& config_;

};  

class RGBResv8BitPerColorPixelWriter : public PixelWriter{ //inherit PixelWrite
    public:
     using PixelWriter::PixelWriter; //inherit constructor
    
    virtual void Write(int x, int y,const PixelColor& c) override {
        auto p = PixelAt(x,y); //PixelAt is protected fucntion
        p[0] = c.r;
        p[1] = c.g;
        p[2] = c.b;
    }
};

class BGRResv8BitPerColorPixelWriter : public PixelWriter{ //inherit PixelWrite
    public:
     using PixelWriter::PixelWriter; //inherit constructor
    
    virtual void Write(int x, int y,const PixelColor& c) override {
        auto p = PixelAt(x,y); //PixelAt is protected fucntion
        p[0] = c.b;
        p[1] = c.g;
        p[2] = c.r;
    }
};

void* operator new(size_t size, void* buf){
    return buf;
}

void operator delete (void* obj) noexcept {}

//these are global variables
char pixel_writer_buf[sizeof(RGBResv8BitPerColorPixelWriter)];
PixelWriter* pixel_writer;

extern "C" void KernelMain(const FrameBufferConfig& frame_buffer_config){ //extern "C" means that we can define function in C language.
    switch (frame_buffer_config.pixel_format) {
        case kPixelRGBResv8BitPerColor:
         pixel_writer = new(pixel_writer_buf) //pointer of pixel writer
         RGBResv8BitPerColorPixelWriter{frame_buffer_config}; //generate instance , not initialization
         break;
        
        case kPixelBGRResv8BitPerColor:
         pixel_writer = new(pixel_writer_buf)
         BGRResv8BitPerColorPixelWriter{frame_buffer_config}; //generate instance , not initialization
         break;
    }

    for (int x = 0; x < frame_buffer_config.horizontal_resolution; ++x){
        for (int y = 0;y < frame_buffer_config.vertical_resolution; ++y){
            pixel_writer -> Write(x,y,{255,255,255});
        }
    }
    for (int x = 0; x < 200; ++x) {
        for (int y = 0; y < 100; ++y) {
            pixel_writer -> Write(x, y, {0, 255, 0});
        }
    }
    while(1) __asm__("hlt"); //inline assemblar inorder to embed assembly language into C language.
}
//"hlt means we want to stop CPU"