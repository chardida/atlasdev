CC         := gcc
CFLAGS     := -std=c99 -Wall -Wextra -O2 -static -D_POSIX_C_SOURCE=200112L

SRC_DIR     := src
BUILD_DIR   := build

IMAGE       := atlas.img
IMAGE_SIZE  := 64    # size in MB
MOUNT_POINT := /mnt
ROOTFS      := rootfs

# Find all .c sources under src/, get relative paths
SRCS      := $(shell find $(SRC_DIR) -type f -name '*.c')
REL_PATHS := $(patsubst $(SRC_DIR)/%,%,$(SRCS))
BINS      := $(patsubst %.c,$(BUILD_DIR)/%,$(REL_PATHS))

.PHONY: all img move run clean

all: $(BINS)

# Compile each source into build/<relative path>, making dirs as needed
$(BUILD_DIR)/%: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $<

# Recreate the full disk image and install entire rootfs/
img:
	@echo "==> Rebuilding $(IMAGE) ($(IMAGE_SIZE)MB))"
	@dd if=/dev/zero of=$(IMAGE) bs=1M count=$(IMAGE_SIZE) status=none
	@mkfs.ext4 -F $(IMAGE)
	@echo "==> Installing full rootfs into $(IMAGE)"
	sudo mount -o loop $(IMAGE) $(MOUNT_POINT)
	sudo cp -a $(ROOTFS)/* $(MOUNT_POINT)/
	sudo umount $(MOUNT_POINT)
	@echo "==> $(IMAGE) rebuilt."

# Mount, copy all freshly-built binaries to their matching paths, then unmount
move: all
	@echo "==> Mounting $(IMAGE)"
	sudo mount -o loop $(IMAGE) $(MOUNT_POINT)
	@echo "==> Copying updated tools"
	@for bin in $(BINS); do \
	  rel=$${bin#$(BUILD_DIR)/}; \
	  dir=$$(dirname "$$rel"); \
	  sudo mkdir -p "$(MOUNT_POINT)/$$dir"; \
	  sudo cp "$$bin" "$(MOUNT_POINT)/$$rel"; \
	done
	sudo umount $(MOUNT_POINT)
	@echo "==> Binaries updated."

# Build, install (via move), then boot the VM
run: move
	qemu-system-x86_64 \
		-kernel kernel/bzImage \
		-append "root=/dev/vda rw console=ttyS0" \
		-drive file=$(IMAGE),if=virtio,format=raw \
		-nographic

# Remove build artifacts
clean:
	rm -rf $(BUILD_DIR)
