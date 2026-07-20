#ifndef GDT_H
#define GDT_H

#include <stdint.h>

typedef struct {
	uint16_t limit_low;	 // no use
	uint16_t base_low;	 // no use
	uint8_t base_middle; // no use
	uint8_t access;
	uint8_t granularity;
	uint8_t base_high; // no use
} __attribute__((packed)) gdt_entry_t;

typedef struct {
	uint16_t limit;
	uint64_t base;
} __attribute__((packed)) gdt_pointer_t;

gdt_entry_t gdt[3] = {
	{0, 0, 0, 0b00000000, 0b00000000, 0}, // null
	{0, 0, 0, 0b10011010, 0b00100000, 0}, // code segment
	{0, 0, 0, 0b10010010, 0b00000000, 0}  // data segment
};
gdt_pointer_t gdt_ptr = {
	sizeof(gdt) - 1,
	(uint64_t) &gdt};

extern void gdt_flush(uint64_t gdt_ptr);

#endif // !GDT_H