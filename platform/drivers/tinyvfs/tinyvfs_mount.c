#include <hal/tinyvfs.h>
#include <hal/blk_dev.h>
#include <stdio.h>

// Mount and intialize all filesystem data
int vfs_mount_init( const struct vfs_mount_point_desc mnt_points[], size_t mnt_size )
{
    int err = blk_initialize();
    if(err) {
        printf("vfs: blk dev initialize error %i\n", err);
        return err;
    }
    for( size_t n=0; n<mnt_size/sizeof(vfs_mount_point_desc_t); ++n ) {
        
    }
    return -1;
}

// Umount and deinit filesystem
int vfs_unmount_deinit()
{
    return -1;
}
