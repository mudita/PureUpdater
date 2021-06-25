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
struct vfs_dir;
struct dirent;
struct stat;
struct statvfs;
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

/** VFS close entry
 * @see man close
 */

int vfs_close(struct vfs_file *zfp);\

/** VFS read entry
 * @see man read
 */

ssize_t vfs_read(struct vfs_file *zfp, void *ptr, size_t size);


/** VFS write entry
 * @see man write
 */
ssize_t vfs_write(struct vfs_file *zfp, const void *ptr, size_t size);
/** VFS seek entry
 * @see man seek
 */
int vfs_seek(struct vfs_file *zfp, off_t offset, int whence);
/** VFS tell entry
 * @see man tell 
 */
off_t vfs_tell(struct vfs_file *zfp);
/** VFS truncate entry
 * @see man truncate 
 */
int vfs_truncate(struct vfs_file *zfp, off_t length);
/** VFS sync entry
 * @see man sync 
 */
int vfs_sync(struct vfs_file *zfp);
/** VFS opendir entry
 * @see man opendir 
 */
int vfs_opendir(struct vfs_dir *zdp, const char *abs_path);
/** VFS readdir entry
 * @see man readdir 
 */
int vfs_readdir(struct vfs_dir *zdp, struct dirent *entry);
/** VFS readdir entry
 * @see man close dir 
 */
int vfs_closedir(struct vfs_dir *zdp);

/** VFS make directory
 * @see man mkdir
 */
int vfs_mkdir(const char *abs_path);

/** VFS unlink
 * @see man unlink
 */
int vfs_unlink(const char *abs_path);

/** VFS rename
 * @see man rename
 */
int vfs_rename(const char *from, const char *to);

/** VFS stat
 * @see man stat
 */
int vfs_stat(const char *abs_path, struct stat *entry);

/** VFS statvfs
 * @see man stat
 */
int vfs_statvfs(const char *abs_path, struct statvfs *stat);

/** Read mount point and its index
 * @param index Mount point index
 * @param name Mount point destiantion
 * @error errno or 0 if success
 */
int vfs_readmount(int *index, const char **name);