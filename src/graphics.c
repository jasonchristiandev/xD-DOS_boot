#include "xddos_boot/graphics.h"
#include <efi.h>

#define Print(x) SystemTable->ConOut->OutputString(SystemTable->ConOut, (x))

void color_bitmask(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info, boot_video_mode_t *mode) {
	if (info->PixelFormat == PixelRedGreenBlueReserved8BitPerColor) {
		mode->red_mask_size = 8;
		mode->red_mask_shift = 0;
		mode->green_mask_size = 8;
		mode->green_mask_shift = 8;
		mode->blue_mask_size = 8;
		mode->blue_mask_shift = 16;
	} else if (info->PixelFormat == PixelBlueGreenRedReserved8BitPerColor) {
		mode->red_mask_size = 8;
		mode->red_mask_shift = 16;
		mode->green_mask_size = 8;
		mode->green_mask_shift = 8;
		mode->blue_mask_size = 8;
		mode->blue_mask_shift = 0;
	} else if (info->PixelFormat == PixelBitMask) {
	}
}

EFI_STATUS graphics_init(EFI_SYSTEM_TABLE *SystemTable, EFI_GRAPHICS_OUTPUT_PROTOCOL *gop) {
	EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

	EFI_STATUS status = SystemTable->BootServices->LocateProtocol(&gop_guid, NULL, (void **) &gop);
	if (EFI_ERROR(status)) return status;

	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info = NULL;
	UINTN info_size = 0;

	status = gop->QueryMode(gop, gop->Mode->Mode, &info_size, &info);
	if (status == EFI_BUFFER_TOO_SMALL) {
		status = gop->QueryMode(gop, gop->Mode->Mode, &info_size, &info);
	}

	return EFI_SUCCESS;
}

EFI_STATUS create_framebuffer(EFI_SYSTEM_TABLE *SystemTable, EFI_GRAPHICS_OUTPUT_PROTOCOL *gop, boot_framebuffer_t *fb) {
	EFI_STATUS status;
	UINT32 max_modes = gop->Mode->MaxMode;

	status = SystemTable->BootServices->AllocatePool(
		EfiLoaderData,
		sizeof(boot_video_mode_t *) * max_modes,
		(void **) &(fb->modes.entries));
	if (EFI_ERROR(status)) return status;

	fb->modes.count = 0;

	for (UINT32 mode_id = 0; mode_id < max_modes; mode_id++) {
		EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info = NULL;
		UINTN size = 0;

		status = gop->QueryMode(gop, mode_id, &size, &info);
		if (EFI_ERROR(status)) continue;

		if (info->PixelFormat == PixelBltOnly) {
			SystemTable->BootServices->FreePool(info);
			continue;
		}

		boot_video_mode_t *mode = NULL;
		status = SystemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(boot_video_mode_t), (void **) &mode);
		if (EFI_ERROR(status)) {
			SystemTable->BootServices->FreePool(info);
			continue;
		}

		mode->width = info->HorizontalResolution;
		mode->height = info->VerticalResolution;
		mode->bpp = 32;
		mode->pitch = info->PixelsPerScanLine * (mode->bpp / 8);
		color_bitmask(info, mode);

		fb->modes.entries[fb->modes.count] = mode;
		fb->modes.count++;

		SystemTable->BootServices->FreePool(info);
	}

	fb->address = gop->Mode->FrameBufferBase;
	fb->size = gop->Mode->FrameBufferSize;
	fb->width = gop->Mode->Info->HorizontalResolution;
	fb->height = gop->Mode->Info->VerticalResolution;
	fb->bpp = 32;
	fb->pitch = gop->Mode->Info->PixelsPerScanLine * (fb->bpp / 8);

	return EFI_SUCCESS;
}