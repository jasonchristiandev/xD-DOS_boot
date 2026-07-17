#include <efi.h>

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
	SystemTable->ConOut->Reset(SystemTable->ConOut, FALSE);
	SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Hello world!\r\n");

	while (1);

	return EFI_SUCCESS;
}