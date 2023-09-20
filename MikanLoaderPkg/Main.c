#include  <Uefi.h>
#include  <Library/UefiLib.h>
#include  <Library/UefiBootServicesTableLib.h>
#include  <Library/PrintLib.h>
#include  <Library/MemoryAllocationLib.h>
#include  <Protocol/LoadedImage.h>
#include  <Protocol/SimpleFileSystem.h>
#include  <Protocol/DiskIo2.h>
#include  <Protocol/BlockIo.h>
#include  <Guid/FileInfo.h>

// #@@range_begin(struct_memory_map)
struct MemoryMap {
  UINTN buffer_size; 
  VOID* buffer;
  UINTN map_size;
  UINTN map_key;
  UINTN descriptor_size;
  UINT32 descriptor_version;
};
// #@@range_end(struct_memory_map)

// #@@range_begin(get_memory_map)
EFI_STATUS GetMemoryMap(struct MemoryMap* map) {
  if (map->buffer == NULL) {
    return EFI_BUFFER_TOO_SMALL;
  }

  map->map_size = map->buffer_size;
  return gBS->GetMemoryMap( //gBSはブートサービス(OSを起動するために必要なサービス)を表すグローバル変数
      &map->map_size, //メモリマップ書き込み用のメモリ領域の大きさ
      (EFI_MEMORY_DESCRIPTOR*)map->buffer, //書き込み用のメモリ領域の先頭ポインタ
      &map->map_key, //メモリマップを識別するための値を書き込む変数
      &map->descriptor_size, //メモリマップの個々の行を
      &map->descriptor_version);
}
// #@@range_end(get_memory_map)

// #@@range_begin(get_memory_type)
const CHAR16* GetMemoryTypeUnicode(EFI_MEMORY_TYPE type) {
  switch (type) {
    case EfiReservedMemoryType: return L"EfiReservedMemoryType";
    case EfiLoaderCode: return L"EfiLoaderCode";
    case EfiLoaderData: return L"EfiLoaderData";
    case EfiBootServicesCode: return L"EfiBootServicesCode";
    case EfiBootServicesData: return L"EfiBootServicesData";
    case EfiRuntimeServicesCode: return L"EfiRuntimeServicesCode";
    case EfiRuntimeServicesData: return L"EfiRuntimeServicesData";
    case EfiConventionalMemory: return L"EfiConventionalMemory";
    case EfiUnusableMemory: return L"EfiUnusableMemory";
    case EfiACPIReclaimMemory: return L"EfiACPIReclaimMemory";
    case EfiACPIMemoryNVS: return L"EfiACPIMemoryNVS";
    case EfiMemoryMappedIO: return L"EfiMemoryMappedIO";
    case EfiMemoryMappedIOPortSpace: return L"EfiMemoryMappedIOPortSpace";
    case EfiPalCode: return L"EfiPalCode";
    case EfiPersistentMemory: return L"EfiPersistentMemory";
    case EfiMaxMemoryType: return L"EfiMaxMemoryType";
    default: return L"InvalidMemoryType";
  }
}
// #@@range_end(get_memory_type)

// #@@range_begin(save_memory_map)
EFI_STATUS SaveMemoryMap(struct MemoryMap* map, EFI_FILE_PROTOCOL* file) {
  CHAR8 buf[256];
  UINTN len;

  CHAR8* header =
    "Index, Type, Type(name), PhysicalStart, NumberOfPages, Attribute\n";
  len = AsciiStrLen(header);
  file->Write(file, &len, header);

  Print(L"map->buffer = %08lx, map->map_size = %08lx\n",
      map->buffer, map->map_size);

  EFI_PHYSICAL_ADDRESS iter;
  int i;
  for (iter = (EFI_PHYSICAL_ADDRESS)map->buffer, i = 0;
       iter < (EFI_PHYSICAL_ADDRESS)map->buffer + map->map_size;
       iter += map->descriptor_size, i++) {
    EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)iter;
    len = AsciiSPrint(
        buf, sizeof(buf),
        "%u, %x, %-ls, %08lx, %lx, %lx\n",
        i, desc->Type, GetMemoryTypeUnicode(desc->Type),
        desc->PhysicalStart, desc->NumberOfPages,
        desc->Attribute & 0xffffflu);
    file->Write(file, &len, buf);
  }

  return EFI_SUCCESS;
}
// #@@range_end(save_memory_map)

EFI_STATUS OpenRootDir(EFI_HANDLE image_handle, EFI_FILE_PROTOCOL** root) {
  EFI_LOADED_IMAGE_PROTOCOL* loaded_image;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fs;

  gBS->OpenProtocol(
      image_handle,
      &gEfiLoadedImageProtocolGuid,
      (VOID**)&loaded_image,
      image_handle,
      NULL,
      EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);

  gBS->OpenProtocol(
      loaded_image->DeviceHandle,
      &gEfiSimpleFileSystemProtocolGuid,
      (VOID**)&fs,
      image_handle,
      NULL,
      EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);

  fs->OpenVolume(fs, root);

  return EFI_SUCCESS;
}

EFI_STATUS OpenGOP(EFI_HANDLE image_handle,
                   EFI_GRAPHICS_OUTPUT_PROTOCOL** gop) {
  UINTN num_gop_handles = 0;
  EFI_HANDLE* gop_handles = NULL;
  gBS->LocateHandleBuffer(
      ByProtocol,
      &gEfiGraphicsOutputProtocolGuid,
      NULL,
      &num_gop_handles,
      &gop_handles);

  gBS->OpenProtocol(
      gop_handles[0],
      &gEfiGraphicsOutputProtocolGuid,
      (VOID**)gop,
      image_handle,
      NULL,
      EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);

  FreePool(gop_handles);

  return EFI_SUCCESS;
}

const CHAR16* GetPixelFormatUnicode(EFI_GRAPHICS_PIXEL_FORMAT fmt) {
  switch (fmt) {
    case PixelRedGreenBlueReserved8BitPerColor:
      return L"PixelRedGreenBlueReserved8BitPerColor";
    case PixelBlueGreenRedReserved8BitPerColor:
      return L"PixelBlueGreenRedReserved8BitPerColor";
    case PixelBitMask:
      return L"PixelBitMask";
    case PixelBltOnly:
      return L"PixelBltOnly";
    case PixelFormatMax:
      return L"PixelFormatMax";
    default:
      return L"InvalidPixelFormat";
  }
}

EFI_STATUS EFIAPI UefiMain(
    EFI_HANDLE image_handle,
    EFI_SYSTEM_TABLE* system_table) {
  Print(L"Hello, Mikan World!\n");

  // #@@range_begin(main)
  CHAR8 memmap_buf[4096 * 4];
  struct MemoryMap memmap = {sizeof(memmap_buf), memmap_buf, 0, 0, 0, 0};
  GetMemoryMap(&memmap);

  EFI_FILE_PROTOCOL* root_dir;
  OpenRootDir(image_handle, &root_dir);

  EFI_FILE_PROTOCOL* memmap_file;
  root_dir->Open(
      root_dir, &memmap_file, L"\\memmap",
      EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);

  SaveMemoryMap(&memmap, memmap_file);
  memmap_file->Close(memmap_file);
  // #@@range_end(main)

  //pixcel
  EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
  OpenGOP(image_handle, &gop); //get GOP if succeed , gop is set to the value
  Print(L"Resolution: %ux%u, Pixel Format: %s, %u pixels/line\n",
      gop->Mode->Info->HorizontalResolution,
      gop->Mode->Info->VerticalResolution,
      GetPixelFormatUnicode(gop->Mode->Info->PixelFormat),
      gop->Mode->Info->PixelsPerScanLine);
  Print(L"Frame Buffer: 0x%0lx - 0x%0lx, Size: %lu bytes\n",
      gop->Mode->FrameBufferBase,
      gop->Mode->FrameBufferBase + gop->Mode->FrameBufferSize,
      gop->Mode->FrameBufferSize);

  // UINT8* frame_buffer = (UINT8*)gop -> Mode -> FrameBufferBase; //head addredd of frame buffer
  // for(UINTN i = 0;i < gop -> Mode -> FrameBufferSize; ++i){
  //   frame_buffer[i] = 255;
  // }

//frame_buffer is overwritten by "kernel ~~".
// import kernel file
  EFI_FILE_PROTOCOL *kernel_file;
  root_dir -> Open(
    root_dir, &kernel_file, L"\\kernel.elf",
    EFI_FILE_MODE_READ,0
  ); //open kernel.elf for read


  //malloc the memory for the opened file.
  //EFI_FILE_INFO struct is in list3.3
  UINTN file_info_size = sizeof(EFI_FILE_INFO) + sizeof(CHAR16) * 12; // extra space is needed for FileName
  UINT8 file_info_buffer[file_info_size];
  kernel_file -> GetInfo(
      kernel_file,&gEfiFileInfoGuid,
      &file_info_size,file_info_buffer);
    
    //file_info_buffer is written by EFI_FILE_INFO_struct

  EFI_FILE_INFO* file_info = (EFI_FILE_INFO*)file_info_buffer;
  UINTN kernel_file_size = file_info -> FileSize;

  //know the size of kernel file!
  //we malloc the memory enough to allocate the file
  EFI_PHYSICAL_ADDRESS kernel_base_addr = 0x100000;
  gBS->AllocatePages(
    AllocateAddress, EfiLoaderData, 
    (kernel_file_size + 0xfff) / 0x1000, &kernel_base_addr);
  //first arg is how allocate memory, second arg is the kind of memory,
  //third arg is size, fourth arg is address

  //read the file
  kernel_file->Read(kernel_file, &kernel_file_size, (VOID*)kernel_base_addr);
  Print(L"Kernel: 0x%0lx (%lu bytes)\n", kernel_base_addr, kernel_file_size);

  //stop the boot service because we want to use kernel
  EFI_STATUS status;
  status = gBS -> ExitBootServices(image_handle,memmap.map_key); //this function returns ERROR if mapkey is not same as current mep key
  if (EFI_ERROR(status)) {
    // Print(L"All done\n");
    status = GetMemoryMap(&memmap);
    if (EFI_ERROR(status)) {
      Print(L"failed to get memory map: %r\n", status);
      while (1);
    }

    status = gBS->ExitBootServices(image_handle, memmap.map_key);
    if (EFI_ERROR(status)) {
      Print(L"Could not exit boot service: %r\n", status);
      while (1);
    }
  }

  //call kernel
  UINT64 entry_addr = *(UINT64*)(kernel_base_addr + 24);

  typedef void EntryPointType(UINT64,UINT64); //we have to call entry point as C language
  EntryPointType* entry_point = (EntryPointType*)entry_addr; //entry_addr is address of entry point
  entry_point(gop->Mode->FrameBufferBase,gop->Mode->FrameBufferSize); //entry_point is an address of a function whose pointer type is defined ad EntryPointType
  //while(1) is icluded in entry_point(),so "ALl done " sohldn't printed.
  
  Print(L"All done\n");
  while (1);
  return EFI_SUCCESS;


  }


  
  


  



