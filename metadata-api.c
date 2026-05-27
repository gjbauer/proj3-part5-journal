#include "metadata-api.h"
#include "btr.h"
#include "directory.h"
#include "hash.h"

int _mknod(DiskInterface *disk, cache *cache, const char *path, mode_t mode, bool write_through)
{
    InodeBtreePair *pair = item_search(disk, cache, path);
    char *parent = parent_path(path, count_l(path));
    char *name = get_name(path);
    if (pair->inode_number)
    {
        free(pair);
        pair = item_search(disk, cache, parent);
        btree_write(disk, cache, pair->btree_block);
        directory_sync_entry(disk, cache, parent, name);
        arc4random_buf(pair, sizeof(struct InodeBtreePair));
        arc4random_buf(parent, strlen(parent));
        arc4random_buf(name, strlen(name));
        free(pair);
        free(parent);
        free(name);
        return 0;
    }
    Inode node;
    int rv = inode_allocate(disk, cache, mode, write_through);
    if (-1 == rv) goto print;
    rv = inode_read(disk, cache, rv, &node);
    if (rv) goto print;
    node.creation_time = time(NULL);
    rv = inode_write(disk, cache, &node, write_through);
    if (rv) goto print;
    rv = directory_add_entry(disk, cache, parent, name, node.inode_number, ( mode & S_IFMT), write_through);
print:
    arc4random_buf(&node, sizeof(struct Inode));
    arc4random_buf(pair, sizeof(struct InodeBtreePair));
    arc4random_buf(parent, sizeof(strlen(parent)));
    arc4random_buf(name, sizeof(char)*strlen(name));
    free(pair);
    free(parent);
    free(name);
    printf("mknod(%s, %04o) -> %d\n", path, mode, rv);
    return rv;
}

int _unlink(DiskInterface *disk, cache *cache, const char *path, bool write_through)
{
    char *parent = parent_path(path, count_l(path));
    char *name = get_name(path);
    int rv = directory_remove_entry(disk, cache, parent, name, write_through);
    arc4random_buf(parent, sizeof(char)*strlen(parent));
    arc4random_buf(name, sizeof(char)*strlen(name));
    free(parent);
    free(name);
    printf("unlink(%s) -> %d\n", path, rv);
    return rv;
}

int _link(DiskInterface *disk, cache *cache, const char *from, const char *to, bool write_through)
{
    int rv = -1;
    InodeBtreePair *from_pair = item_search(disk, cache, from);
    Inode from_inode;
    inode_read(disk, cache, from_pair->inode_number, &from_inode);
    char *to_parent = parent_path(to, count_l(to));
    char *to_name = get_name(to);
    rv = directory_add_entry(disk, cache, to_parent, to_name, from_pair->inode_number, (FileType) ( from_inode.mode & S_IFMT), write_through);
    if (!rv)
    {
        from_inode.reference_count++;
        inode_write(disk, cache, &from_inode, write_through);
    }
    
    arc4random_buf(from_pair, sizeof(struct InodeBtreePair));
    arc4random_buf(to_parent, sizeof(char)*strlen(to_parent));
    arc4random_buf(to_name, sizeof(char)*strlen(to_name));
    free(from_pair);
    free(to_parent);
    free(to_name);
    printf("link(%s => %s) -> %d\n", from, to, rv);
    return rv;
}

int _chmod(DiskInterface *disk, cache *cache, const char *path, mode_t mode, bool write_through)
{
    int rv = -1;
    InodeBtreePair *pair = item_search(disk, cache, path);
    Inode node;
    rv = inode_read(disk, cache, pair->inode_number, &node);
    if (rv) return rv;
    node.mode = mode;
    rv = inode_write(disk, cache, &node, write_through);
    arc4random_buf(pair, sizeof(struct InodeBtreePair));
    arc4random_buf(&node, sizeof(struct Inode));
    free(pair);
    printf("chmod(%s, %04o) -> %d\n", path, mode, rv);
    return rv;
}

int _truncate(DiskInterface *disk, cache *cache, const char *path, off_t size, bool write_through)
{
    int rv = 0;
    InodeBtreePair *pair = item_search(disk, cache, path);
    Inode inode;
    inode_read(disk, cache, pair->inode_number, &inode);

    if (size < inode.size) {
        // Need to free blocks - but this should be idempotent
        // Calculate which blocks to free based on new vs old size
        uint64_t old_blocks = (inode.size + USABLE_BLOCK_SIZE - 1) / USABLE_BLOCK_SIZE;
        uint64_t new_blocks = (size + USABLE_BLOCK_SIZE - 1) / USABLE_BLOCK_SIZE;

        for (uint64_t i = new_blocks; i < old_blocks; i++) {
            uint64_t physical_block = 0;
            if (!inode_get_block(disk, cache, &inode, i, &physical_block) && physical_block) {
                free_page(disk, cache, physical_block);
                inode_set_block(disk, cache, &inode, i, 0);
            }
        }
    }

    inode.size = size;
    inode_write(disk, cache, &inode, write_through);
    // TODO: Free empty indirect and double-indirect blocks when empty
    printf("truncate(%s, %lld bytes) -> %d\n", path, size, rv);
    return rv;
}

int _set_block(DiskInterface *disk, cache *cache, int64_t inode_number,
               uint64_t block_index, uint64_t physical_block)
{
    int rv = -1;
    Inode inode;

    if (inode_read(disk, cache, inode_number, &inode) != 0) {
        fprintf(stderr, "ERROR: Could not read inode %lld!!\n", inode_number);
        return -1;
    }

    if (block_index < 12)
    {
        inode.direct_blocks[block_index] = physical_block;
        rv = 0;
    }
    else if (block_index <= ( USABLE_BLOCK_SIZE / sizeof(uint64_t) ) )
    {
        if (!inode.indirect_block)
        {
            inode.indirect_block = alloc_page(disk, cache);
            if (!inode.indirect_block) return rv;
            block_type_t *block_type = get_block(disk, cache, inode.inode_number, inode.indirect_block);
            *block_type = BLOCK_TYPE_DATA;
            inode_write(disk, cache, &inode, true);
            inode_read(disk, cache, inode.inode_number, &inode);
            uint64_t *sind = (uint64_t*)( block_type + 1 );
            memset(sind, 0, USABLE_BLOCK_SIZE);
            write_block(disk, cache, block_type, inode.inode_number, inode.indirect_block);
            disk_write_block(disk, inode.indirect_block, block_type);
        }
        block_type_t *block_type = get_block(disk, cache, inode.inode_number, inode.indirect_block);
        uint64_t *sind = (uint64_t*)( block_type + 1 );
        sind += ( block_index - 12 );
        *sind = physical_block;
        write_block(disk, cache, block_type, inode.inode_number, inode.indirect_block);
        disk_write_block(disk, inode.indirect_block, block_type);
        block_type = get_block(disk, cache, inode.inode_number, physical_block);
        *block_type = BLOCK_TYPE_DATA;
        write_block(disk, cache, block_type, inode.inode_number, physical_block);
        disk_write_block(disk, physical_block, block_type);
        rv = 0;
    }

    if (inode_write(disk, cache, &inode, true) != 0) {
        return -1;
    }

    return rv;
}
