#include "disk.h"
#include "cache.h"
#include <stdio.h>

typedef enum TANSACTION_TYPE
{
    BTREE_UPDATE,
    PAGE_UPDATE,
    INODE_UPDATE,
    SUPERBLOCK_UPDATE,
} TANSACTION_TYPE;

