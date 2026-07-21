#include "xddos_boot/protocol.h"
#include <efi.h>

EFI_STATUS graphics_init(EFI_SYSTEM_TABLE *SystemTable, EFI_GRAPHICS_OUTPUT_PROTOCOL **gop);
EFI_STATUS create_framebuffer(EFI_SYSTEM_TABLE *SystemTable, EFI_GRAPHICS_OUTPUT_PROTOCOL **gop, boot_framebuffer_t *fb);