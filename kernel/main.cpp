#include <cstdint>

extern "C" void KernelMain(uint64_t frame_buffer_base,
                           uint64_t frame_buffer_size){ //extern "C" means that we can define function in C language.
    uint8_t* frame_buffer = reinterpret_cast<uint8_t*>(frame_buffer_base); //cast to pointer type
    for(uint64_t i = 0;i < frame_buffer_size;++i){
        frame_buffer[i] = i % 256;
    } 
    while(1) __asm__("hlt"); //inline assemblar inorder to embed assembly language into C language.
}
//"hlt means we want to stop CPU"