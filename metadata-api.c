#include "metadata-api.h"

int _mknod(DiskInterface *disk, cache *cache, const char *path, mode_t mode, bool write_through)
{
    Inode node;
    char *parent = parent_path(path, count_l(path));
    char *name = get_name(path);
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
    arc4random_buf(parent, sizeof(char)*strlen(parent));
    arc4random_buf(name, sizeof(char)*strlen(name));
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
    // TODO: Implement truncate
    printf("truncate(%s, %lld bytes) -> %d\n", path, size, rv);
    return rv;
}
