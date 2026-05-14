#include "config.h"
#include "superblock.h"
#include "journal.h"
#include "metadata-api.h"

void initialize_journal_entry(DiskInterface *disk, cache *cache, journal_entry_t *entry)
{
    Superblock sb;
    journal_entry_t *prev_entry;
    block_type_t *block_type;

    superblock_read(disk, cache, &sb);

    block_type = (block_type_t*)get_block(disk, cache, 0, sb.journal_start + sb.journal_head);
    prev_entry = (journal_entry_t*)(block_type + 1);
    sync_entry(disk, cache, prev_entry);

    if (sb.journal_head == ( calculate_journal_size(&sb) - 1 ) ) sb.journal_head = 0;
    else sb.journal_head++;

    superblock_write(disk, cache, &sb, true);

    memcpy(prev_entry, entry, sizeof(struct journal_entry_t));
}

void sync_entry(DiskInterface *disk, cache *cache, journal_entry_t *entry)
{
    switch (entry->type)
    {
        case UNITIALIZED:
            break;
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
    entry->synced = true;
}

void sync_journal(DiskInterface *disk, cache *cache)
{
    Superblock sb;
    journal_entry_t *prev_entry;
    block_type_t *block_type;

    superblock_read(disk, cache, &sb);

    for (int i = 0; i < calculate_journal_size(&sb); i++)
    {
        block_type = (block_type_t*)get_block(disk, cache, 0, sb.journal_start + sb.journal_head);
        prev_entry = (journal_entry_t*)(block_type + 1);
        sync_entry(disk, cache, prev_entry);

        if (sb.journal_head == ( calculate_journal_size(&sb) - 1 ) ) sb.journal_head = 0;
        else sb.journal_head++;
    }

    superblock_write(disk, cache, &sb, true);
}
