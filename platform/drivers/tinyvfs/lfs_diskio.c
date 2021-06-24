#include <hal/blk_dev.h>
#include <lfs.h>
#include <stdio.h>
#include <errno.h>
#include <prv/tinyvfs/lfs_diskio.h>

#define min(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b;       \
})

//! Default lfs block size if doesn't exists
static const size_t c_lfs_block_size = 32U * 1024U;

struct io_context
{
    int disk;             //! Disk handle
    size_t erase_size;      //! Erase size
    size_t sector_size;     //! Sector size
};

static int errno_to_lfs(int error)
{
    if (error >= 0) {
        return LFS_ERR_OK;
    }
    switch (error) {
        default:
        case -EIO: /* Error during device operation */
            return LFS_ERR_IO;
        case -EFAULT: /* Corrupted */
            return LFS_ERR_CORRUPT;
        case -ENOENT: /* No directory entry */
            return LFS_ERR_NOENT;
        case -EEXIST: /* Entry already exists */
            return LFS_ERR_EXIST;
        case -ENOTDIR: /* Entry is not a dir */
            return LFS_ERR_NOTDIR;
        case -EISDIR: /* Entry is a dir */
            return LFS_ERR_ISDIR;
        case -ENOTEMPTY: /* Dir is not empty */
            return LFS_ERR_NOTEMPTY;
        case -EBADF: /* Bad file number */
            return LFS_ERR_BADF;
        case -EFBIG: /* File too large */
            return LFS_ERR_FBIG;
        case -EINVAL: /* Invalid parameter */
            return LFS_ERR_INVAL;
        case -ENOSPC: /* No space left on device */
            return LFS_ERR_NOSPC;
        case -ENOMEM: /* No more memory available */
            return LFS_ERR_NOMEM;
    }
}

static int setup_lfs_config(struct lfs_config *cfg, size_t sector_size, size_t part_sectors_count, size_t block_size)
{
    if(block_size>0) {
        cfg->block_size = block_size;
    }
    else {
        cfg->block_size = c_lfs_block_size;
        printf("vfs_lfs warn: Mount block size not specified using default value\n");
    }
    cfg->block_cycles = 512;
    cfg->block_count = 0; // Read later from super block
    const uint64_t total_siz = (uint64_t)sector_size * (uint64_t)part_sectors_count;
    if (total_siz % cfg->block_size)
    {
        printf("vfs_lfs: Block size doesn't match partition size\n");
        return -ERANGE;
    }
    cfg->block_count = total_siz / cfg->block_size - 1;
    cfg->lookahead_size = min(131072U, ((cfg->block_count >> 3U) + 1U) << 3U);
    cfg->read_size = cfg->block_size;
    cfg->prog_size = cfg->block_size;
    cfg->cache_size = cfg->block_size;
    printf("vfs_lfs info: Block count %u block size %u", (unsigned)cfg->block_count, (unsigned)cfg->block_size);
    return 0;
}

//! Write little FS block
static int lfs_read(const struct lfs_config *lfsc, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size)
{
    // LOG_DEBUG("lfs_read_req(block=%u off=%u size=%u", unsigned(block), unsigned(off), unsigned(size));
    struct io_context* ctx = (struct io_context *)lfsc->context;
    if (!ctx) {
        return LFS_ERR_IO;
    }
    const uint64_t lba = (uint64_t)((lfsc->block_size) * block) / ctx->sector_size;
    if (off % ctx->sector_size) {
        printf("vfs_lfs: Partial offset not supported\n");
        return LFS_ERR_IO;
    }
    const size_t lba_sz = size / ctx->sector_size;
    if (size % ctx->sector_size) {
        printf("vfs_lfs: Bounary read sz error\n");
        return LFS_ERR_IO;
    }
    const int err = blk_read(ctx->disk, lba, lba_sz, buffer);
    if (err) {
        printf("vfs_lfs: Sector read error %i\n", err);
    }
    return errno_to_lfs(err);
}

// Read littlefs block
static int lfs_prog( const struct lfs_config *lfsc, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size )
{
    struct io_context* ctx = (struct io_context*)lfsc->context;
    if (!ctx)
    {
        return LFS_ERR_IO;
    }
    const uint64_t lba = ((uint64_t)(lfsc->block_size) * block) / ctx->sector_size;
    if (off % ctx->sector_size)
    {
        printf("vfs_lfs: Partial offset not supported\n");
        return LFS_ERR_IO;
    }
    const size_t lba_sz = size / ctx->sector_size;
    const int err = blk_write(ctx->disk, lba, lba_sz, buffer );
    if (err) {
        printf("vfs_lfs: Sector read error %i\n", err);
    }
    return errno_to_lfs(err);
}

//! Erase is not supported due to large block in emmc
static int lfs_erase(const struct lfs_config *lfsc, lfs_block_t block)
{
    return LFS_ERR_OK;
}

//! Sync is not supported we don't cache anything
static int lfs_sync(const struct lfs_config *lfsc)
{
    return LFS_ERR_OK;
}

/** Append volume to the selected partition block
 * @param[in] blkdev block device for append 
 * @param[out] lfsc Block configuration structure
 * @return Error code on success otherwise error
 */
int vfs_lfs_append_volume( int blkdev, struct lfs_config* lfsc ) 
{
    if (!lfsc)
    {
        return -EINVAL;
    }
    blk_dev_info_t dinfo;
    blk_partition_t pinfo;
    int err = blk_info( blkdev, &dinfo );
    if(err < 0 ) {
        printf("vfs_lfs: Unable to read dev info %i\n", err);
        return err;
    }
    err = blk_get_partition(blkdev, &pinfo);
    if(err < 0 ) {
        printf("vfs_lfs: Unable to read dev info %i\n", err);
        return err;
    }

    struct io_context* ctx = calloc(1, sizeof(struct io_context));
    ctx->erase_size = dinfo.erase_group;
    ctx->sector_size = dinfo.sector_size;
    ctx->disk = blkdev;
    lfsc->context = ctx;
    lfsc->read = lfs_read;
    lfsc->prog = lfs_prog;
    lfsc->erase = lfs_erase;
    lfsc->sync = lfs_sync;
    lfsc->context = ctx;

    return setup_lfs_config(lfsc, dinfo.sector_size, pinfo.num_sectors, pinfo.erase_blk);
}


/** Remove volume from the LFS and destroy 
 * the context
 * @param lfsc Lfs configuration structure
 */
void vfs_lfs_remove_volume(struct lfs_config *lfsc)
{
    if (!lfsc)
    {
        return;
    }
    if (lfsc->context)
    {
        free(lfsc->context);
    }
}
