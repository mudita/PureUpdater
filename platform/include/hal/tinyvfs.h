#pragma once

#include <stddef.h>
#include <sys/types.h>
/* Tiny virtual filesystem implementation 
 * Note: currently only the emmc user disc is supported
 * due to mappings in the fat driver
 * Example init usage
 * vfs_mount_point_desc fstab[] = 
 * {
 *     { .disk = blldev_emmcuser, .partition = 1, .type = vfs_fs_fat, "/muditaos" },
 *     { .disk = blldev_emmcuser, .partition = 3, .type = vfs_fs_littlefs, "/user" },
 * };
 * error = vfs_mount_init( fstab, sizeof fstab );
 */

//! Filesystem type
typedef enum vfs_filesystem_type {
    vfs_fs_fat,
    vfs_fs_littlefs
} vfs_filesystem_type_t;

//! Tiny VFS mount point
typedef struct vfs_mount_point_desc {
    short disk;                     //! Mount the disc 
    short partition;                //! Partition number on the EMMC disc
    vfs_filesystem_type_t type;     //! Filesystem type
    const char* mount_point;        //! Base mount point
} vfs_mount_point_desc_t;

struct vfs_filesystem_ops;
struct vfs_mount;
struct vfs_file;
/** 
 * Initialize the mini virtual file system by the mount points given as an argument
 * @param[in] mnt_desc Mount point table
 * @param[in] mnt_len  Size of of the mount point table
 * @return Error status
 */
int vfs_mount_init( const struct vfs_mount_point_desc mnt_desc[], size_t mnt_size );

/** Uninitialize the virtual filesystem and unmount all mount points
 * @return Error status
 */
int vfs_unmount_deinit();

/** 
 * Register the filesystem
 * @param type Filesystem type
 * @param fops File system operation structure
 * @return error otherwvise success
 */
int vfs_register_filesystem(int type, const struct vfs_filesystem_ops *fops);


/** Mount filesystem 
 *  Mount the selected filesystem;
 * @param  mp Mount point
 */
int vfs_mount(struct vfs_mount *mp);

/** Umount the selected filesystem
 * @param mp Mount point
 */
int vfs_unmount(struct vfs_mount *mp);

/** VFS open entry
 * @see man open
 */
int vfs_open(struct vfs_file *zfp, const char *file_name, int flags, mode_t mode);

