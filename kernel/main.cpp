#include <cstdint>
#include <cstddef>
#include <cstdio>

#include "frame_buffer_config.hpp"
#include "graphics.hpp"
#include "font.hpp"
#include "console.hpp"
#include "pci.hpp"

void operator delete(void* obj) noexcept {}

const PixelColor kDesktopBGColor{45, 118, 237};
const PixelColor kDesktopFGColor{255, 255, 255};

//the shape of mouse cursor
const int kMouseCursorWidth = 15;
const int kMouseCursorHeight = 24;
const char mouse_cursor_shape[kMouseCursorHeight][kMouseCursorWidth + 1] = {
    "@              ",
    "@@             ",
    "@.@            ",
    "@..@           ",
    "@...@          ",
    "@....@         ",
    "@.....@        ",
    "@......@       ",
    "@.......@      ",
    "@........@     ",
    "@.........@    ",
    "@..........@   ",
    "@...........@  ",
    "@............@ ",
    "@......@@@@@@@@",
    "@......@       ",
    "@....@@.@      ",
    "@...@ @.@      ",
    "@..@   @.@     ",
    "@.@    @.@     ",
    "@@      @.@    ",
    "@       @.@    ",
    "         @.@   ",
    "         @@@   ",
};

char pixel_writer_buf[sizeof(RGBResv8BitPerColorPixelWriter)];
PixelWriter* pixel_writer;

char console_buf[sizeof(Console)];
Console* console; //global variabl!!

int printk(const char* format, ...) { //allocatable args
    va_list ap;
    int result;
    char s[1024];

    va_start(ap,format);
    result = vsprintf(s,format,ap);
    va_end(ap);

    console -> PutString(s);
    return result;
}

extern "C" void KernelMain(const FrameBufferConfig& frame_buffer_config){
    switch(frame_buffer_config.pixel_format){
        case kPixelRGBResv8BitPerColor:
            pixel_writer = new(pixel_writer_buf)RGBResv8BitPerColorPixelWriter{frame_buffer_config};
            break;
        case kPixelBGRResv8BitPerColor:
            pixel_writer = new(pixel_writer_buf)BGRResv8BitPerColorPixelWriter{frame_buffer_config};
            break;
    }

    const int kFrameWidth = frame_buffer_config.horizontal_resolution;
    const int kFrameHeight = frame_buffer_config.vertical_resolution;

    FillRectangle(*pixel_writer,
                        {0,0},
                        {kFrameWidth, kFrameHeight - 50},
                        kDesktopBGColor);
        
    FillRectangle(*pixel_writer,
                        {0,kFrameHeight - 50},
                        {kFrameWidth, 50},
                        {1,8,17});
        
    FillRectangle(*pixel_writer,
                    {0, kFrameHeight - 50},
                    {kFrameWidth / 5, 50},
                    {80, 80, 80});

    DrawRectangle(*pixel_writer,
                    {10, kFrameHeight - 40},
                    {30, 30},
                    {160, 160, 160});


    // to use console as an global variable(it's defined at l20)
    console = new(console_buf) Console(*pixel_writer, kDesktopFGColor, kDesktopBGColor); //allocate console in global area

    printk("Welcome to MikanOS!\n");

    auto err = pci::ScanAllBus();
    Log(kDebug, "ScanAllBus: %s\n", err.Name());

    for (int i = 0;i < pci::num_device; ++i) {
        const auto& dev = pci::devices[i];
        auto vendor_id = pci::ReadVendorId(dev);
        auto class_code = pci::ReadClassCode(dev.bus, dev.device, dev.function);
        Log(kDebug, "%d.%d.%d: vend %04x, class %08x, head %02x\n",
        dev.bus, dev.device, dev.function,
        vendor_id, class_code, dev.header_type);
    }
    
    pci::Device* xhc_dev = nullptr;
    for (int i = 0; i < pci::num_device ; ++i) {
        if (pci::devices[i].class_code.Match(0x0cu, 0x03u, 0x30u)) { //it means xHCl(ref p151)
            xhc_dev = &pci::devices[i];

            if (0x8086 == pci::ReadVendorId(*xhc_dev)) { //adopt made intel 
                break;
            }
        }
    }

    if(xhc_dev) {
        Log(kInfo, "xHC has been found: %d.%d.%d\n",
            xhc_dev -> bus, xhc_dev -> device, xhc_dev -> function);
    }

    const WithError<uint64_t> xhc_bar = pci::ReadBar(*xhc_dev, 0);
    Log(kDebug, "REadBar: %s\n", xhc_bar.error.Name());
    const uint64_t xhc_mmio_base = xhc_bar.value & ~static_cast <uint64_t> (0xf);
    Log(kDebug, "xHC mmio_base = %08lx\n", xhc_mmio_base);

    while (1) __asm__("hlt");
}

