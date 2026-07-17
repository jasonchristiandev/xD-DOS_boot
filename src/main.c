#include <efi.h>
#include <elf.h>

#define WIDEN2(x) L##x
#define WIDEN(x) WIDEN2(x)

#ifndef COMMIT_HASH
#define COMMIT_HASH "unknown"
#endif

#define Print(x) SystemTable->ConOut->OutputString(SystemTable->ConOut, (x))

EFI_FILE_HANDLE GetVolume(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
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
	Print(L"xD-DOS (Extended Drive - Disk Operating System) Bootloader (" WIDEN(COMMIT_HASH) L")\r\n"
																							 L"> https://github.com/jasonchristiandev/xD-DOS_boot\r\n"
																							 L"> Maintained by Jason Christian.\r\n\n"
																							 L"> Press any key to continue...\r\n\n");

	EFI_FILE_HANDLE volume = GetVolume(ImageHandle, SystemTable);
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
			Print(L"Failed to allocate pool for kernel\r\n");
			for (;;) {}
		}

		status = file->GetInfo(file, &guid, &size, info);
		if (EFI_ERROR(status)) {
			Print(L"Failed to get kernel data\r\n");
			for (;;) {}
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
			Print(L"Failed to get free file info pool\r\n");
			for (;;) {}
		}

		EFI_MEMORY_DESCRIPTOR *memmap = NULL;
		UINTN memmap_key;
		UINTN desc_size;
		UINT32 desc_ver;
		UINTN memmap_size = 0;

		status = SystemTable->BootServices->GetMemoryMap(&memmap_size, NULL, &memmap_key, &desc_size, &desc_ver);
		if (EFI_ERROR(status)) {
			Print(L"Failed to get memory map\r\n");
			for (;;) {}
		}

		memmap_size += (2 * desc_size);
		status = SystemTable->BootServices->AllocatePool(EfiLoaderData, memmap_size, (void **) &memmap);
		if (EFI_ERROR(status)) {
			Print(L"Failed to get allocate pool for memory map\r\n");
			for (;;) {}
		}

		status = SystemTable->BootServices->GetMemoryMap(&memmap_size, memmap, &memmap_key, &desc_size, &desc_ver);
		if (EFI_ERROR(status)) {
			Print(L"Failed to get memory map\r\n");
			for (;;) {}
		}

		status = SystemTable->BootServices->ExitBootServices(ImageHandle, memmap_key);
		if (EFI_ERROR(status)) {
			Print(L"Failed to exit boot services\r\n");
			for (;;) {}
		}

		typedef void (*KernelEntry)(void);
		KernelEntry jump = (KernelEntry) entry;
		// jump();
	}

	return EFI_SUCCESS;
}