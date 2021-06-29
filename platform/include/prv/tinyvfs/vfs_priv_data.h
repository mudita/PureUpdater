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

