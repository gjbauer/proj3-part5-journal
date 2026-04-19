#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disk.h"
#include "cache.h"
#include "btr.h"
#include "superblock.h"
#include "inode.h"
#include "hash.h"

int main(int argc, char *argv[])
{
    int rv = -1;
    char volume_name[32] = "UNTITLED";
    if (argc % 2 != 0)
    {
        fprintf(stderr, "ERROR: Invalid number of arguments!!\n");
        goto err;
    }
    if (argc > 2)
    {
        if (!strcmp(argv[1], "-l"))
        {
            strncpy(volume_name, argv[2], 32);
        }
    }
    DiskInterface* disk = disk_open(argv[--argc]);
    cache *cache = NULL;
    if (disk_format(disk, cache, volume_name)) goto err;
    return 0;
err:
    fprintf(stderr, "ERROR: Could not format disk!!\n");
    return rv;
}
