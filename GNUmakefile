CFLAGS = -g
FUSE_FLAGS = -lfuse -D_FILE_OFFSET_BITS=64
RM_FILES = my.img mkfs.nbtrfs fuse mnt test.log
# Cache files are optimized out of compilation for mkfs & for builds with CACHE_DISABLED macro...
COMMON_FILES = bitmap.c btr.c cache.c disk.c dl.c fl.c gdl.c hash.c lru.c pci.c superblock.c inode.c string.c directory.c

UNAME_S := $(shell uname -s)

# Linux-specific flags
ifeq ($(UNAME_S), Linux)
    CFLAGS += -lbsd
endif

# macOS-specific flags
ifeq ($(UNAME_S), Darwin)
    RM_FILES += *.dSYM
endif

all: fuse

fuse: test_image
	clang $(CFLAGS) -o fuse $(COMMON_FILES) fuse.c $(FUSE_FLAGS)

sanitize: test_image
	clang $(CFLAGS) -fsanitize=address -o fuse $(COMMON_FILES) fuse.c $(FUSE_FLAGS)

format: mkfs fuse
	./mkfs.nbtrfs my.img
	
mount:
	./fuse -s -f mnt my.img

unmount:
	sudo umount mnt
	
mkfs:
	clang $(CFLAGS) -o mkfs.nbtrfs $(COMMON_FILES) mkfs.c -DCACHE_DISABLED

test_image:
		dd if=/dev/zero of=my.img bs=1M count=20

clean:
	rm -rf $(RM_FILES)

open:
	nvim -p *.h *.c

.PHONY: clean open
