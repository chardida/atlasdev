CC         := gcc
CFLAGS     := -std=c99 -Wall -Wextra -O2 -static -D_POSIX_C_SOURCE=200112L

SRC_DIR     := src
BUILD_DIR   := build

IMAGE       := atlas.img
IMAGE_SIZE  := 64    # size in MB
MOUNT_POINT := mnt
ROOTFS      := $(BUILD_DIR)

# Find all main.c sources under src/, get relative paths
MAINS      := $(shell find $(SRC_DIR) -type f -name 'main.c')
MAIN_REL_PATHS := $(patsubst $(SRC_DIR)/%,%,$(dir $(MAINS)))
MAIN_BINS  := $(patsubst %/,$(BUILD_DIR)/%,$(MAIN_REL_PATHS))

.PHONY: all img move run clean

all: img

# Compile each main.c into build/<relative path without trailing slash>, making dirs as needed
$(BUILD_DIR)/%: $(SRC_DIR)/%/main.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $<

# Recreate the full disk image and install entire rootfs/
img: $(MAIN_BINS)
	@echo "==> Rebuilding $(IMAGE) ($(IMAGE_SIZE)MB))"
	@dd if=/dev/zero of=$(IMAGE) bs=1M count=$(IMAGE_SIZE) status=none
	@mkfs.ext4 -F $(IMAGE)
	@echo "==> Installing full rootfs into $(IMAGE)"
	@mkdir -p $(MOUNT_POINT) $(ROOTFS)
	@find $(ROOTFS) -type f -exec chmod +x {} \;
	sudo mount -o loop $(IMAGE) $(MOUNT_POINT)
	sudo cp -a $(ROOTFS)/* $(MOUNT_POINT)/
	sudo umount $(MOUNT_POINT)
	@echo "==> $(IMAGE) rebuilt."

# Mount, copy all freshly-built main binaries to their matching paths, then unmount
move: all
	@echo "==> Mounting $(IMAGE)"
	sudo mount -o loop $(IMAGE) $(MOUNT_POINT)
	@echo "==> Copying updated main binaries"
	@for bin in $(MAIN_BINS); do \
	  rel=$${bin#$(BUILD_DIR)/}; \
	  dir=$$(dirname "$$rel"); \
	  sudo mkdir -p "$(MOUNT_POINT)/$$dir"; \
	  sudo cp "$$bin" "$(MOUNT_POINT)/$$rel"; \
	done
	sudo umount $(MOUNT_POINT)
	@echo "==> Main binaries updated."

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
