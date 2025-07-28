# Compiler settings
CC         := gcc
CFLAGS     := -std=c99 -Wall -Werror -Wextra -O2 -static -D_POSIX_C_SOURCE=200112L

# Directory structure
SRC_DIR     := src
BUILD_DIR   := build
IMAGE       := atlas.img
IMAGE_SIZE  := 64    # size in MB
MOUNT_POINT := mnt
ROOTFS      := $(BUILD_DIR)

# Find all main.c paths and map to binary paths
MAINS           := $(shell find $(SRC_DIR) -type f -name 'main.c')
MAIN_REL_PATHS  := $(patsubst $(SRC_DIR)/%,%,$(dir $(MAINS)))
MAIN_BINS       := $(patsubst %,$(BUILD_DIR)/%main,$(MAIN_REL_PATHS))

.PHONY: all img move run clean mount umount

all: img

# Compile each main.c into build/<path>/main
$(BUILD_DIR)/%main: $(SRC_DIR)/%main.c
    @mkdir -p $(dir $@)
    $(CC) $(CFLAGS) -o $@ $<

# Rebuild disk image and install entire rootfs
img: $(MAIN_BINS)
    @echo "==> Building $(IMAGE) ($(IMAGE_SIZE)MB)"
    @dd if=/dev/zero of=$(IMAGE) bs=1M count=$(IMAGE_SIZE) status=none
    @mkfs.ext4 -F $(IMAGE)
    @echo "==> Copying rootfs contents"
    @mkdir -p $(MOUNT_POINT)
    @sudo mount -o loop $(IMAGE) $(MOUNT_POINT)
    @find $(ROOTFS) -type f -exec sudo cp --parents {} $(MOUNT_POINT)/ \;
    @sudo sync &&
