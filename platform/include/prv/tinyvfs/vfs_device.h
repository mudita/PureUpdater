#pragma once

#include <stddef.h>
#include <sys/types.h>

struct vfs_file;
struct vfs_dir;
struct vfs_mount;
struct stat;
struct statvfs;
struct dirent;

//! Device filesystem operation structure
struct vfs_filesystem_ops {
    // File operations
    int (*open)(struct vfs_file *filp, const char *fs_path, int flags, int mode);
    ssize_t (*read)(struct vfs_file *filp, void *dest, size_t nbytes);
    ssize_t (*write)(struct vfs_file *filp, const void *src, size_t nbytes);
    int (*lseek)(struct vfs_file *filp, off_t off, int whence);
    off_t (*tell)(struct vfs_file *filp);
    int (*truncate)(struct vfs_file *filp, off_t length);
    int (*sync)(struct vfs_file *filp);
    int (*close)(struct vfs_file *filp);
    //int (*fchmod)(struct vfs_file *filp, mode_t mode);

    // Directory operations
    int (*opendir)(struct vfs_dir *dirp, const char *fs_path);
    int (*readdir)(struct vfs_dir *dirp, struct dirent *entry);
    int (*closedir)(struct vfs_dir *dirp);

    // Filesystem level operations
    int (*mount)(struct vfs_mount *mountp);
    int (*unmount)(struct vfs_mount *mountp);
    int (*unlink)(struct vfs_mount *mountp, const char *name);
    int (*rename)(struct vfs_mount *mountp, const char *from, const char *to);
    int (*mkdir)(struct vfs_mount *mountp, const char *name);
    int (*stat)(struct vfs_mount *mountp, const char *path, struct stat *entry);
    int (*statvfs)(struct vfs_mount *mountp, const char *path, struct statvfs *stat);
    int (*chmod)(struct vfs_mount *mountp, const char *path, mode_t mode);
};
