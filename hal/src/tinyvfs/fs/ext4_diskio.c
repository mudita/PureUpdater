#include <hal/blk_dev.h>
#include <ext4_config.h>
#include <ext4_blockdev.h>
#include <ext4_errno.h>
#include <prv/tinyvfs/ext4_diskio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

//! Internal io context
struct io_context
{
    struct ext4_blockdev_iface ifc;     //! Block interface
    int disk;                           //! Disk handle
};

//! Ext4 open blockdev
static int io_open(struct ext4_blockdev *bdev)
{
    (void)(bdev);
    return 0;
}
//! Ext4 close blockdev
static int io_close(struct ext4_blockdev *bdev)
{
    (void)(bdev);
    return 0;
}

// Ext4 write sectors
static int io_write(struct ext4_blockdev *bdev, const void *buf, uint64_t blk_id, uint32_t blk_cnt)
{
    struct io_context* ctx = bdev->bdif->p_user;
    if(!ctx) {
        return EIO;
    }
    return -blk_write(ctx->disk, blk_id, blk_cnt, buf);
}

//Ext4 read sectors
static int io_read(struct ext4_blockdev *bdev, void *buf, uint64_t blk_id, uint32_t blk_cnt)
{
    struct io_context* ctx = bdev->bdif->p_user;
    if(!ctx) {
        return EIO;
    }
    return -blk_read(ctx->disk, blk_id, blk_cnt, buf);
}


/** Append volume to the selected partition block
 * @param[in] blkdev block device for append
 * @param[out] bdev Ext4 block device objecvt
 * @return Error code on success otherwise error
 */
int vfs_ext4_append_volume(int blkdev, struct ext4_blockdev **bdev)
{
    if (!bdev)
    {
        return -EINVAL;
    }
    blk_dev_info_t dinfo;
    blk_partition_t pinfo;
    int err = blk_info(blkdev, &dinfo);
    if (err < 0)
    {
        printf("vfs_lfs: Unable to read dev info %i\n", err);
        return err;
    }
    err = blk_get_partition(blkdev, &pinfo);
    if (err < 0)
    {
        printf("vfs_lfs: Unable to read dev info %i\n", err);
        return err;
    }
    if (pinfo.type != blk_part_type_ext4)
    {
        printf("vfs_lfs: Unable to mount non EXT4 part %02x\n", pinfo.type);
    }
    *bdev = calloc(1, sizeof(**bdev));
    if(!(*bdev)) {
        return -ENOMEM;
    }
    struct io_context *ctx = calloc(1, sizeof(struct io_context));
    if(!ctx) {
        free(*bdev);
        return -ENOMEM;
    }
    ctx->disk = blkdev;
    ctx->ifc.open         = io_open;
    ctx->ifc.bread        = io_read;
    ctx->ifc.bwrite       = io_write;
    ctx->ifc.close        = io_close;
    ctx->ifc.ph_bbuf      = malloc(dinfo.sector_size);
    if(!ctx->ifc.ph_bbuf) {
        free(ctx);
        free(*bdev);
        return -ENOMEM;
    }
    ctx->ifc.ph_bcnt      = pinfo.num_sectors;
    ctx->ifc.ph_bsize     = dinfo.sector_size;
    ctx->ifc.p_user       = ctx;
    (*bdev)->bdif        = &ctx->ifc;
    (*bdev)->part_offset = 0;
    (*bdev)->part_size   = (uint64_t)dinfo.sector_size * pinfo.num_sectors;
    return 0;
}

/** Remove volume from the LFS and destroy
 * the context
 * @param bdev Block device configuration structure
 */
void vfs_ext4_remove_volume(struct ext4_blockdev *bdev)
{
    if(!bdev) {
        return;
    }
    struct io_context *ctx = bdev->bdif->p_user;
    free(ctx->ifc.ph_bbuf);
    free(ctx);
    free(bdev);
}

