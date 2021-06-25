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
    const struct vfs_mount *mp;     // VFS mount poiunt
    int flags;        // Open flags
    mode_t mode;      // Structure representing mode flags
};

//! Structure representing file
struct vfs_dir {
    void *dirp;     //Directory pointer
    const struct vfs_mount *mp;   //Mount point
};
