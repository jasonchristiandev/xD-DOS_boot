#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

// memmap
typedef enum : uint64_t {
	BOOT_MEMMAP_USABLE = 0,
	BOOT_MEMMAP_RESERVED = 1,
	BOOT_MEMMAP_ACPI_RECLAIMABLE = 2,
	BOOT_MEMMAP_ACPI_NVS = 3,
	BOOT_MEMMAP_BAD_MEMORY = 4,
	BOOT_MEMMAP_BOOTLOADER_RECLAIMABLE = 5,
	BOOT_MEMMAP_EXECUTABLE_AND_MODULES = 6,
	BOOT_MEMMAP_FRAMEBUFFER = 7,
	BOOT_MEMMAP_RESERVED_MAPPED = 8
} boot_memmap_type_t;

typedef struct {
	uint64_t base;
	uint64_t length;
	boot_memmap_type_t type;
} boot_memmap_entry_t;

typedef struct {
	uint64_t count;
	boot_memmap_entry_t **entries;
} boot_memmap_t;

// framebuffer
typedef struct {
	uint64_t width;
	uint64_t height;
	uint64_t pitch;
	uint16_t bpp;
	uint8_t red_mask_size;
	uint8_t red_mask_shift;
	uint8_t green_mask_size;
	uint8_t green_mask_shift;
	uint8_t blue_mask_size;
	uint8_t blue_mask_shift;
} boot_video_mode_t;

typedef struct {
	uint64_t count;
	boot_video_mode_t **entries;
} boot_video_modes_t;

typedef struct {
	void *address;
	uint64_t size;
	uint64_t width;
	uint64_t height;
	uint64_t pitch;
	uint16_t bpp;
	boot_video_modes_t modes;
} boot_framebuffer_t;

typedef struct {
	uint64_t count;
	boot_framebuffer_t **entries;
} boot_framebuffers_t;

// executable
typedef struct {
	uint64_t phys;
	uint64_t virt;
} boot_executable_address_t;

typedef struct {
	void *address;
	uint64_t size;
} boot_executable_file_t;

// boot_info
typedef struct {
	boot_memmap_t *memmap;
	uint64_t hhdm;
	boot_framebuffers_t *framebuffers;
	boot_executable_address_t *exeaddr;
	boot_executable_file_t *exefile;
} boot_info_t;

#endif // !PROTOCOL_H