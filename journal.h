#include "disk.h"
#include "cache.h"
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>
#include <stdint.h>

typedef enum transaction_type_t
{
    MKNOD,
    UNLINK,
    LINK,
    CHMOD,
    TRUNCATE,
} transaction_type_t;

typedef struct journal_entry_t
{
    transaction_type_t type;
    int position;
    bool synced;
    union
    {
        struct
        {
            char path[PATH_MAX];
            mode_t mode;
        } mknod;
        struct
        {
            char path[PATH_MAX];
        } unlink;
        struct
        {
            char from[PATH_MAX];
            char to[PATH_MAX];
        } link;
        struct
        {
            char path[PATH_MAX];
            mode_t mode;
        } chmod;
        struct
        {
            char path[PATH_MAX];
            off_t size;
        } truncate;
    };
} journal_entry_t;

typedef struct journal_header_t
{
    int head_position;
} journal_header_t;

void set_journal_head(DiskInterface *disk, cache *cache, int head_position);

void initialize_journal_entry(DiskInterface *disk, cache *cache, transaction_type_t type);

void sync_entry(DiskInterface *disk, cache *cache, int position);

void sync_journal(DiskInterface *disk, cache *cache);

