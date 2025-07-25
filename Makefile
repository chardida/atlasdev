CC       := gcc
CFLAGS   := -std=c99 -Wall -Wextra -O2 -static -D_POSIX_C_SOURCE=200112L

SRC_DIR     := src
BUILD_DIR   := build

IMAGE       := atlas.img
IMAGE_SIZE  := 64    # size in MB
MOUNT_POINT := /mnt
ROOTFS      := rootfs

ROOTFS_BIN  := $(MOUNT_POINT)/bin
ROOTFS_SBIN := $(MOUNT_POINT)/sbin

SRCS := $(wildcard $(SRC_DIR)/*.c)
BINS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%,$(SRCS))

.PHONY: all img move run clean

all: $(BINS)

# 1) Compile each source file into a statically-linked binary
$(BUILD_DIR)/%: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $<

# 2) Create or recreate the disk image from scratch
img:
	@echo "==> Rebuilding $(IMAGE) ($(IMAGE_SIZE)MB)..."
	@dd if=/dev/zero of=$(IMAGE) bs=1M count=$(IMAGE_SIZE) status=none
	@mkfs.ext4 -F $(IMAGE)
	@echo "==> Installing full rootfs into $(IMAGE)..."
	sudo mount -o loop $(IMAGE) $(MOUNT_POINT)
	sudo cp -a $(ROOTFS)/* $(MOUNT_POINT)/
	sudo umount $(MOUNT_POINT)
	@echo "==> $(IMAGE) rebuilt."

# 3) Install just the updated binaries into the existing image
move: all
	@echo "==> Mounting $(IMAGE)..."
	sudo mount -o loop $(IMAGE) $(MOUNT_POINT)
	@echo "==> Copying updated tools..."
	sudo cp $(BUILD_DIR)/init      $(ROOTFS_SBIN)/init
	sudo cp $(BUILD_DIR)/sh        $(ROOTFS_BIN)/sh
	sudo cp $(BUILD_DIR)/poweroff  $(ROOTFS_BIN)/poweroff
	sudo umount $(MOUNT_POINT)
	@echo "==> Binaries updated."

# 4) Build, install (move), and launch the VM
run: move
	qemu-system-x86_64 \
		-kernel kernel/bzImage \
		-append "root=/dev/vda rw console=ttyS0" \
		-drive file=$(IMAGE),if=virtio,format=raw \
		-nographic

# 5) Cleanup build artifacts
clean:
	rm -rf $(BUILD_DIR)
