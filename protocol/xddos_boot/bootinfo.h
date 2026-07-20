#include <stdint.h>

// memmap
typedef struct {
	uint64_t base;
	uint64_t length;
	uint64_t type;
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

// boot_info
typedef struct {
	boot_memmap_t *memmap;
	uint64_t hhdm;
	boot_framebuffers_t *framebuffers;
} boot_info_t;