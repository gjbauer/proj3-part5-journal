#include "config.h"
#include "superblock.h"
#include "journal.h"
#include "metadata-api.h"

void initialize_journal_entry(DiskInterface *disk, cache *cache, journal_entry_t *entry)
{
    printf("initialize_journal_entry called with type: 0x%x\n", entry->type);

    Superblock sb;
    journal_entry_t *prev_entry;
    block_type_t *block_type;

    superblock_read(disk, cache, &sb);

    uint64_t journal_block = sb.journal_start + sb.journal_head;
    printf("Reading journal block %llu\n", journal_block);

    block_type = (block_type_t*)get_block(disk, cache, 0, journal_block);

    if (*block_type != BLOCK_TYPE_JOURNAL) {
        fprintf(stderr, "ERROR: Block %llu is not a journal block! type=0x%x\n", journal_block, *block_type);
        // Don't proceed - journal is corrupted
        return;
    }

    prev_entry = (journal_entry_t*)(block_type + 1);

    if (prev_entry->type != UNINITIALIZED && !prev_entry->synced) {
        printf("Found existing journal entry, syncing...\n");
        sync_entry(disk, cache, prev_entry);
    }

    if (sb.journal_head == ( calculate_journal_size(&sb) - 1 ) ) sb.journal_head = 0;
    else sb.journal_head++;

    superblock_write(disk, cache, &sb, true);

    memcpy(prev_entry, entry, sizeof(struct journal_entry_t));
    write_block(disk, cache, block_type, 0, journal_block);
    printf("Journal entry written, new head = %llu\n", sb.journal_head);
}

void sync_entry(DiskInterface *disk, cache *cache, journal_entry_t *entry)
{
    switch (entry->type)
    {
        case UNINITIALIZED:
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
        case WRITE:
            _set_block(disk, cache, entry->write.inode_number, entry->write.block_index, entry->write.physical_block);
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
        int head = (sb.journal_head == ( calculate_journal_size(&sb) - 1 ) ) ? 0 : sb.journal_head + 1;
        block_type = (block_type_t*)get_block(disk, cache, 0, sb.journal_start + head);
        prev_entry = (journal_entry_t*)(block_type + 1);

        if (*block_type != BLOCK_TYPE_JOURNAL) {
            fprintf(stderr, "ERROR: Block %llu is not a journal block! type=0x%x\n", sb.journal_start + sb.journal_head, *block_type);
            // Don't proceed - journal is corrupted
            return;
        }

        if (prev_entry->type != UNINITIALIZED && !prev_entry->synced) {
            printf("Found existing journal entry, syncing...\n");
            sync_entry(disk, cache, prev_entry);
        }

        if (sb.journal_head == ( calculate_journal_size(&sb) - 1 ) ) sb.journal_head = 0;
        else sb.journal_head++;
    }

    superblock_write(disk, cache, &sb, true);
}
