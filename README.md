# atlas

## Build System

This project uses a GitHub Actions to execute the Makefile automatically, and on the cloud for compatability on any system.

### How it works
- Recursively searches `src/` for all `main.c` files.
- Each `main.c` is built as a binary in `build/<relative path without trailing slash>` (e.g., `src/bin/shutdown/main.c` → `build/bin/shutdown`).
- All built binaries are installed into an ext4 disk image (`atlas.img`) as the root filesystem.

### Main Makefile Targets
- `make` or `make all`: Builds all binaries and creates the disk image.
- `make img`: Builds all binaries and installs them into the disk image.
- `make move`: Mounts the image and copies updated binaries into it.
- `make run`: Boots the image in QEMU using the specified kernel.
- `make clean`: Removes all build artifacts.

### Example
If you have:
```
src/bin/shutdown/main.c
src/sbin/init/main.c
```
They will be built as:
```
build/bin/shutdown
build/sbin/init
```

### Usage
1. Build everything:
   ```bash
   make
   ```
2. Boot the image in QEMU:
   ```bash
   make run
   ```
3. Clean build artifacts:
   ```bash
   make clean
   ```

### Notes
- All binaries are set as executable before being copied to the image.
- The image is mounted at `mnt/` during installation.
- The kernel image is expected at `kernel/bzImage`.
