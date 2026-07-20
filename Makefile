ROOT_DIR = $(abspath ..)
include ../settings.mk

HASH = $(shell git rev-parse --short HEAD)
TAG = $(shell git describe --tags --exact-match 2>/dev/null)
COMMIT_HASH = $(if $(TAG),$(TAG),$(HASH))
CC = x86_64-w64-mingw32-gcc

CFLAGS += -I/usr/include/efi -I/usr/include/efi/x86_64 -I$(BOOTLOADER_DIR)/protocol
CFLAGS += -ffreestanding -fno-stack-protector -mno-red-zone -fshort-wchar -DCOMMIT_HASH=\"$(COMMIT_HASH)\"

SRCS = $(wildcard $(SRC_DIR)/*.c)
ASMS = $(wildcard $(ASM_DIR)/*.s)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.c.o, $(SRCS)) $(patsubst $(ASM_DIR)/%.s, $(BUILD_DIR)/%.s.o, $(ASMS))

.PHONY: all run clean

all: $(BOOTX64_EFI)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.c.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo " [CC] $<"
	@$(CC) $(CFLAGS) -c $< -o $@ -DDEBUG=$(VERBOSE)

$(BUILD_DIR)/%.s.o: $(ASM_DIR)/%.s | $(BUILD_DIR)
	@echo " [ASM] $<"
	@$(ASM) -f win64 $< -o $@

$(BOOTX64_EFI): $(OBJS) | $(BUILD_DIR)
	@echo " [LD] Linking..."
	@$(CC) -nostdlib -Wl,-subsystem,10 -e efi_main $(OBJS) -o $(BOOTX64_EFI)

clean:
	rm -rf $(BUILD_DIR)

-include $(OBJS:.o=.d)