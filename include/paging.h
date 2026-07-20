#ifndef PAGING_H
#define PAGING_H

#include <efi.h>
#include <stdint.h>

#define PAGE_SIZE 4096

typedef enum : uint16_t {
	PTE_PRESENT = 1 << 0,
	PTE_READWRITE = 1 << 1,
	PTE_USER = 1 << 2,
	PTE_WRITETHROUGH = 1 << 3,
	PTE_CACHEDISABLE = 1 << 4,
	PTE_ACCESSED = 1 << 5,
	PTE_DIRTY = 1 << 6,
	PTE_PAGESIZE = 1 << 7,
	PTE_GLOBAL = 1 << 8,
	// PTE_ = 1 << 9,
	// PTE_ = 1 << 10,
	// PTE_ = 1 << 11
} pte_flag_t;

void paging_init(EFI_SYSTEM_TABLE *SystemTable);
void map_table(EFI_SYSTEM_TABLE *SystemTable, uint64_t virt, uint64_t phys, uint64_t flags);

#endif // !PAGING_H