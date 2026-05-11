#include "superblock.h"
#include "journal.h"
#include "metadata-api.h"

void set_journal_head(DiskInterface *disk, cache *cache, int head_position);

void initialize_journal_entry(DiskInterface *disk, cache *cache, journal_entry_t *entry)
{
}

void sync_entry(DiskInterface *disk, cache *cache, journal_entry_t *entry)
{
    switch (entry->type)
    {
        case MKNOD:
            _mknod(disk, cache, entry->mknod.path, entry->mknod.mode, true);
            break;
        case UNLINK:
            _unlink(disk, cache, entry->unlink.path, true);
            break;
        case LINK:
            _link(disk, cache, entry->link.from, entry->link.to, true);
            break;
        case CHMOD:
            _chmod(disk, cache, entry->chmod.path, entry->chmod.mode, true);
            break;
        case TRUNCATE:
            _truncate(disk, cache, entry->truncate.path, entry->truncate.size, true);
            break;
    }
}

void sync_journal(DiskInterface *disk, cache *cache);
