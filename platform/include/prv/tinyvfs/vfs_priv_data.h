#pragma once

#include <sys/types.h>
#include <stddef.h>

struct vfs_mount {                          
    int type;             
    const char *mnt_point;          
    void *fs_data;                                                                                                             
    int storage_dev;                                                                                                         
    size_t mountp_len;                                       
    const struct vfs_filesystem_ops *fs;                                                                                         
};        

//! Structure representing file
struct vfs_file {
    void *filep;       // File pointer
    const struct vfs_mount *mp;     // VFS mount point
    int flags;        // Open flags
    mode_t mode;      // Structure representing mode flags
};

struct vfs_mount_entry;

//! Structure representing file
struct vfs_dir {
    struct vfs_mount_entry *next_mnt; //Directory pointer
    const struct vfs_mount *mp;   //Mount point
    void *dirp;                    // Structure directory pointer
};

