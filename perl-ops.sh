#!/bin/bash

# setup.sh - Run this first to create and mount the filesystem
# ./setup.sh

# Then run this script to perform all operations
# ./reproduce.sh

set -e  # Exit on error
set -x  # Print each command as it runs

MOUNT_POINT="mnt"

echo "=== Basic Tests ==="

# Create files with content
echo "hello, one" > "$MOUNT_POINT/one.txt"
echo "hello, two" > "$MOUNT_POINT/two.txt"

# Create a 2k file (40 chars * 50 = 2000 chars)
long_string="=This string is fourty characters long.="
long_content=$(printf "%s" "$long_string%.0s" {1..50})
echo "$long_content" > "$MOUNT_POINT/2k.txt"

echo "=== Less Basic Tests ==="

# Remove one.txt
rm "$MOUNT_POINT/one.txt"

# Rename two.txt to abc.txt
mv "$MOUNT_POINT/two.txt" "$MOUNT_POINT/abc.txt"

# Create hard link
ln "$MOUNT_POINT/abc.txt" "$MOUNT_POINT/def.txt"

# Remove the original (abc.txt) leaving the hard link
rm "$MOUNT_POINT/abc.txt"

# Create directory
mkdir "$MOUNT_POINT/foo"

# Copy def.txt to foo/abc.txt
cp "$MOUNT_POINT/def.txt" "$MOUNT_POINT/foo/abc.txt"

# Create a 40k file (40 chars * 1000 = 40000 chars)
huge_content=$(printf "%s" "$long_string%.0s" {1..1000})
echo "$huge_content" > "$MOUNT_POINT/40k.txt"

# Create nested directories
mkdir -p "$MOUNT_POINT/dir1/dir2/dir3/dir4/dir5"

# Create a file in the nested directory
echo "hello there" > "$MOUNT_POINT/dir1/dir2/dir3/dir4/dir5/hello.txt"

# Create numbers directory
mkdir "$MOUNT_POINT/numbers"

# Create 300 numbered files
for ii in {1..300}; do
    echo "$ii" > "$MOUNT_POINT/numbers/$ii.num"
done

echo "Created 300 files"
ls "$MOUNT_POINT/numbers" | wc -l

# Delete even-numbered files (2, 4, 6, ..., 300)
for ii in {1..150}; do
    xx=$((ii * 2))
    rm "$MOUNT_POINT/numbers/$xx.num"
done

echo "After deletion:"
ls "$MOUNT_POINT/numbers" | wc -l

echo "=== All operations complete ==="
