#include <hal/tinyvfs.h>
#include <hal/blk_dev.h>
#include <prv/tinyvfs/vfs_littlefs.h>
#include <prv/tinyvfs/vfs_vfat.h>
#include <prv/tinyvfs/vfs_priv_data.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/syslimits.h>

//! Module context
struct vfs_mount_ctx
{
    struct vfs_mount *mps;
    size_t num_mps;
};

// Global mount point storage
static struct vfs_mount_ctx ctx;

// Mount and intialize all filesystem data
int vfs_mount_init(const struct vfs_mount_point_desc mnt_points[], size_t mnt_size)
{
    if (ctx.num_mps > 0)
    {
        printf("vfs: Subsystem already initialized\n");
        return -EEXIST;
    }
    int err = blk_initialize();
    if (err)
    {
        printf("vfs: blk dev initialize error %i\n", err);
        return err;
    }
    err = vfs_priv_enable_littlefs_filesystem();
    if (err)
    {
        printf("vfs: Unable to register littlefs filesystem");
        return err;
    }
    err = vfs_priv_enable_vfat_filesystem();
    if (err)
    {
        printf("vfs: Unable to register vfat filesystem");
        return err;
    }
    const size_t nitems = mnt_size / sizeof(vfs_mount_point_desc_t);
    ctx.mps = calloc(nitems, sizeof(struct vfs_mount));
    if (!ctx.mps)
    {
        printf("vfs: Unable to allocate memory for mount points\n");
        return -ENOMEM;
    }
    ctx.num_mps = nitems;
    for (size_t n = 0; n < nitems; ++n)
    {
        const vfs_mount_point_desc_t *desc = &mnt_points[n];
        struct vfs_mount *mp = &ctx.mps[n];
        mp->mnt_point = strndup(desc->mount_point, PATH_MAX);
        mp->type = desc->type;
        const int device = blk_disk_handle(desc->disk, desc->partition);
        const int err = vfs_mount(mp, device);
        if (err)
        {
            printf("vfs: Unable to mount dev %i:%i on %s error %i\n", desc->disk, desc->partition, desc->mount_point, err);
            free(ctx.mps);
            ctx.num_mps = 0;
            ctx.mps = NULL;
            return err;
        }
    }
    return 0;
}

// Umount and deinit filesystem
int vfs_unmount_deinit()
{
    if (ctx.num_mps == 0)
    {
        printf("vfs: Already deinitialized\n");
        return -EINVAL;
    }
    for (size_t n = 0; n < ctx.num_mps; ++n)
    {
        struct vfs_mount *mp = &ctx.mps[n];
        const int err = vfs_unmount(mp);
        if (err)
        {
            printf("vfs: Unable unmount device errno %i\n", err);
            return err;
        }
    }
    free(ctx.mps);
    ctx.num_mps = 0;
    ctx.mps = NULL;
    vfs_unregister_all_filesystems();
    return 0;
}
