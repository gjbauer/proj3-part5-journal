#include "superblock.h"
#include "journal.h"

void set_journal_head(DiskInterface *disk, cache *cache, int head_position);

void initialize_journal_entry(DiskInterface *disk, cache *cache, journal_entry_t *entry)
{
}

void sync_entry(DiskInterface *disk, cache *cache, journal_entry_t *entry)
{
}

void sync_journal(DiskInterface *disk, cache *cache);
