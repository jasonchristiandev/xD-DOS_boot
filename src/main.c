#include "xddos_boot/gdt.h"
#include "xddos_boot/graphics.h"
#include "xddos_boot/paging.h"
#include <efi.h>
#include <elf.h>

#define WIDEN2(x) L##x
#define WIDEN(x) WIDEN2(x)

#ifndef COMMIT_HASH
#define COMMIT_HASH "unknown"
#endif

#define Print(x) SystemTable->ConOut->OutputString(SystemTable->ConOut, (x))

extern void jump(void *gdt_ptr, void *stack, void *entry, boot_info_t *info);

EFI_FILE_HANDLE get_volume(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
	EFI_LOADED_IMAGE *loaded = NULL;
	EFI_GUID lip_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *io_volume;
	EFI_GUID fs_guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
	EFI_FILE_HANDLE volume;
	EFI_STATUS status;

	status = SystemTable->BootServices->HandleProtocol(ImageHandle, &lip_guid, (void **) &loaded);
	if (EFI_ERROR(status)) {
		Print(L"Protocol error\r\n");
		for (;;) {}
	}

	status = SystemTable->BootServices->HandleProtocol(loaded->DeviceHandle, &fs_guid, (VOID *) &io_volume);
	if (EFI_ERROR(status)) {
		Print(L"Protocol error\r\n");
		for (;;) {}
	}

	io_volume->OpenVolume(io_volume, &volume);
	return volume;
}

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
	EFI_STATUS status;

	SystemTable->ConOut->Reset(SystemTable->ConOut, FALSE);
	Print(L"xD-DOS Bootloader (" WIDEN(COMMIT_HASH) L")\r\n"
													L"> https://github.com/jasonchristiandev/xD-DOS_boot\r\n"
													L"> Maintained by Jason Christian.\r\n\n"
													L" [Press any key to continue...]\r\n\n");

	UINTN index;
	EFI_EVENT events[1];
	events[0] = SystemTable->ConIn->WaitForKey;
	SystemTable->BootServices->WaitForEvent(1, events, &index);
	SystemTable->ConOut->ClearScreen(SystemTable->ConOut);

	EFI_FILE_HANDLE volume = get_volume(ImageHandle, SystemTable);
	EFI_FILE_HANDLE file;
	status = volume->Open(volume, &file, L"\\boot\\kernel.elf", EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY | EFI_FILE_HIDDEN | EFI_FILE_SYSTEM);
	if (EFI_ERROR(status)) {
		Print(L"Failed to open kernel file\r\n");
		for (;;) {}
	}

	UINTN size = 0;
	EFI_FILE_INFO *info = NULL;
	EFI_GUID guid = EFI_FILE_INFO_ID;
	status = file->GetInfo(file, &guid, &size, NULL);

	if (status == EFI_BUFFER_TOO_SMALL) {
		status = SystemTable->BootServices->AllocatePool(EfiLoaderData, size, (void **) &info);
		if (EFI_ERROR(status)) {
			Print(L"Failed to allocate pool for kernel info\r\n");
			for (;;) {}
		}

		status = file->GetInfo(file, &guid, &size, info);
		if (EFI_ERROR(status)) {
			Print(L"Failed to get kernel data\r\n");
			for (;;) {}
		}
	}

	VOID *base = NULL;
	status = SystemTable->BootServices->AllocatePool(EfiLoaderData, info->FileSize, &base);
	if (EFI_ERROR(status)) {
		Print(L"Failed to allocate pool for kernel\r\n");
		for (;;) {}
	}

	UINTN read_size = info->FileSize;
	status = file->Read(file, &read_size, base);
	if (EFI_ERROR(status)) {
		Print(L"Failed to get kernel file size\r\n");
		for (;;) {}
	}

	Elf64_Ehdr *ehdr = (Elf64_Ehdr *) base;
	UINT64 entry = ehdr->e_entry;

	status = SystemTable->BootServices->FreePool(info);
	if (EFI_ERROR(status)) {
		Print(L"Failed to free file info pool\r\n");
		for (;;) {}
	}

	paging_init(SystemTable);

	Elf64_Phdr *phdr = (Elf64_Phdr *) ((UINT8 *) base + ehdr->e_phoff);
	for (UINT16 i = 0; i < ehdr->e_phnum; i++) {
		if (phdr[i].p_type == PT_LOAD) {
			UINT64 virt = phdr[i].p_vaddr;
			UINT64 phys = virt >= 0xFFFFFFFF80000000 ? virt - 0xFFFFFFFF80000000 : virt;

			VOID *dest = (VOID *) phys;
			VOID *src = (VOID *) ((UINT8 *) base + phdr[i].p_offset);

			SystemTable->BootServices->CopyMem(dest, src, phdr[i].p_filesz);

			if (phdr[i].p_memsz > phdr[i].p_filesz) {
				SystemTable->BootServices->SetMem((UINT8 *) dest + phdr[i].p_filesz, phdr[i].p_memsz - phdr[i].p_filesz, 0);
			}

			UINT64 start = virt & ~0xFFFULL;
			UINT64 end = (virt + phdr[i].p_memsz + 0xFFFULL) & ~0xFFFULL;
			UINT64 cur = phys & ~0xFFFULL;

			for (UINT64 addr = start; addr < end; addr += 4096) {
				map_table(SystemTable, addr, cur, PTE_READWRITE);
				cur += 4096;
			}
		}
	}

	EFI_MEMORY_DESCRIPTOR *memmap = NULL;
	UINTN memmap_key = 0;
	UINTN desc_size = 0;
	UINT32 desc_ver = 0;
	UINTN memmap_size = 0;

	SystemTable->BootServices->GetMemoryMap(&memmap_size, NULL, &memmap_key, &desc_size, &desc_ver);
	memmap_size += (2 * desc_size);
	status = SystemTable->BootServices->AllocatePool(EfiLoaderData, memmap_size, (void **) &memmap);
	if (EFI_ERROR(status)) {
		Print(L"Failed to allocate pool for memory map\r\n");
		for (;;) {}
	}

	EFI_PHYSICAL_ADDRESS stack;
	SystemTable->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, 4, &stack);
	void *stack_top = (void *) (stack + 0x4000);

	// framebuffer
	EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = NULL;
	boot_framebuffer_t fb;
	graphics_init(SystemTable, &gop);
	create_framebuffer(SystemTable, &gop, &fb);

	while (TRUE) {
		UINTN current_size = memmap_size;
		status = SystemTable->BootServices->GetMemoryMap(&current_size, memmap, &memmap_key, &desc_size, &desc_ver);
		if (!EFI_ERROR(status)) {
			status = SystemTable->BootServices->ExitBootServices(ImageHandle, memmap_key);
			if (!EFI_ERROR(status)) {
				break;
			}
		}
	}

	// boot info
	boot_framebuffers_t fbs;
	fbs.count = 1;
	boot_framebuffer_t *fbp = &fb;
	fbs.entries = &fbp;
	boot_info_t bootinfo;
	bootinfo.hhdm = HHDM_OFFSET;
	bootinfo.framebuffers = &fbs;

	jump(&gdt_ptr, stack_top, (void *) entry, &bootinfo);

	return EFI_SUCCESS;
}
