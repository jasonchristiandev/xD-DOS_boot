#include "paging.h"
#include <efi.h>
#include <stdint.h>

#define Print(x) SystemTable->ConOut->OutputString(SystemTable->ConOut, (x))

const uint64_t hhdm_offset = 0xFFFF800000000000ULL;

typedef struct {
	uint64_t entries[512];
} __attribute__((packed)) page_table_t;

page_table_t *pml4;

EFI_PHYSICAL_ADDRESS alloc_page(EFI_SYSTEM_TABLE *SystemTable) {
	EFI_PHYSICAL_ADDRESS phys;
	SystemTable->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &phys);
	return phys;
}

void alloc_entry(EFI_SYSTEM_TABLE *SystemTable, page_table_t *table, uint16_t idx) {
	EFI_PHYSICAL_ADDRESS phys = alloc_page(SystemTable);
	SystemTable->BootServices->SetMem((void *) phys, 4096, 0);

	table->entries[idx] = (uint64_t) phys | PTE_PRESENT | PTE_READWRITE;
}

void map_table(EFI_SYSTEM_TABLE *SystemTable, uint64_t virt, uint64_t phys, uint64_t flags) {
	if ((virt & 0xFFF) != 0) return; // check alignment

	uint16_t pml4e_i = (virt >> 39) & 0b111111111;
	uint16_t pdpte_i = (virt >> 30) & 0b111111111;
	uint16_t pde_i = (virt >> 21) & 0b111111111;
	uint16_t pte_i = (virt >> 12) & 0b111111111;

	page_table_t *table = pml4;
	uint64_t entry;

	// pml4 to pdpt
	if (!(table->entries[pml4e_i] & PTE_PRESENT)) {
		alloc_entry(SystemTable, table, pml4e_i);
	}
	entry = table->entries[pml4e_i];
	table = (page_table_t *) ((uint64_t) entry & 0x000FFFFFFFFFF000);

	// pdpt to pd
	if (!(table->entries[pdpte_i] & PTE_PRESENT)) {
		alloc_entry(SystemTable, table, pdpte_i);
	}
	entry = table->entries[pdpte_i];
	table = (page_table_t *) ((uint64_t) entry & 0x000FFFFFFFFFF000);

	// pd to pt
	if (!(table->entries[pde_i] & PTE_PRESENT)) {
		alloc_entry(SystemTable, table, pde_i);
	}
	entry = table->entries[pde_i];
	table = (page_table_t *) ((uint64_t) entry & 0x000FFFFFFFFFF000);

	// pt entry
	table->entries[pte_i] = phys | flags | PTE_PRESENT;
}

void map_table_huge(EFI_SYSTEM_TABLE *SystemTable, uint64_t virt, uint64_t phys, uint64_t flags) {
	if ((virt & 0xFFF) != 0) return; // check alignment

	uint16_t pml4e_i = (virt >> 39) & 0b111111111;
	uint16_t pdpte_i = (virt >> 30) & 0b111111111;
	uint16_t pde_i = (virt >> 21) & 0b111111111;

	page_table_t *table = pml4;
	uint64_t entry;

	// pml4 to pdpt
	if (!(table->entries[pml4e_i] & PTE_PRESENT)) {
		alloc_entry(SystemTable, table, pml4e_i);
	}
	entry = table->entries[pml4e_i];
	table = (page_table_t *) ((uint64_t) entry & 0x000FFFFFFFFFF000);

	// pdpt to pd
	if (!(table->entries[pdpte_i] & PTE_PRESENT)) {
		alloc_entry(SystemTable, table, pdpte_i);
	}
	entry = table->entries[pdpte_i];
	table = (page_table_t *) ((uint64_t) entry & 0x000FFFFFFFFFF000);

	// pd entry
	table->entries[pde_i] = (phys & 0x000FFFFFFFFFF000) | flags | PTE_PRESENT | PTE_PAGESIZE;
}

void identity_map_range(EFI_SYSTEM_TABLE *SystemTable, uint64_t start, uint64_t end, uint64_t flags) {
	start &= ~0xFFFULL;
	end = (end + 0xFFFULL) & ~0xFFFULL;

	for (uint64_t addr = start; addr < end; addr += 4096) {
		map_table(SystemTable, addr, addr, flags);
	}
}

void paging_init(EFI_SYSTEM_TABLE *SystemTable) {
	// allocate page table
	pml4 = (page_table_t *) alloc_page(SystemTable);
	SystemTable->BootServices->SetMem((void *) pml4, 4096, 0);

	// identity map bootloader and stack
	for (uint64_t i = 0; i < 0x100000000ULL; i += 0x200000) {
		map_table_huge(SystemTable, i, i, PTE_READWRITE);
	}

	// hhdm
	for (uint64_t i = 0; i < 0x100000000ULL; i += 0x200000) {
		map_table_huge(SystemTable, hhdm_offset + i, i, PTE_READWRITE);
	}

	// stack
	uint64_t stack;
	uint64_t stack_top = (uint64_t) &stack;
	uint64_t stack_bottom = stack_top - (32 * 1024); // 32kb
	identity_map_range(SystemTable, stack_bottom, stack_top, PTE_READWRITE);

	__asm__ __volatile__("mov %0, %%cr3" ::"r"((uint64_t) pml4) : "memory");
}
