CC       := gcc
CFLAGS   := -std=c99 -Wall -Wextra -O2 -static

SRC_DIR     := src
BUILD_DIR   := build
IMAGE       := mydistro.img
MOUNT_POINT := /mnt
ROOTFS_BIN  := $(MOUNT_POINT)/bin
ROOTFS_SBIN := $(MOUNT_POINT)/sbin

SRCS := $(wildcard $(SRC_DIR)/*.c)
BINS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%,$(SRCS))

.PHONY: all move run clean

all: $(BINS)

# Compile each source file into a statically-linked binary
$(BUILD_DIR)/%: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $<

# Install fresh binaries into the disk image
move: all
	@echo "Mounting $(IMAGE) on $(MOUNT_POINT)..."
	sudo mount -o loop $(IMAGE) $(MOUNT_POINT)
	@echo "Copying init and sh into rootfs..."
	sudo cp $(BUILD_DIR)/init $(ROOTFS_SBIN)/init
	sudo cp $(BUILD_DIR)/sh   $(ROOTFS_BIN)/sh
	sudo cp $(BUILD_DIR)/poweroff   $(ROOTFS_BIN)/poweroff
	@echo "Unmounting $(IMAGE)..."
	sudo umount $(MOUNT_POINT)
	@echo "Update complete."

# Build, install, and launch the VM
run: move
	qemu-system-x86_64 \
		-kernel kernel/bzImage \
		-append "root=/dev/vda rw console=ttyS0" \
		-drive file=$(IMAGE),if=virtio,format=raw \
		-nographic

# Remove all build artifacts
clean:
	rm -rf $(BUILD_DIR)
