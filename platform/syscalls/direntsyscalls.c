#include <errno.h>
#include <hal/tinyvfs.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

struct __dirstream
{
    struct vfs_dir dirh;
    size_t position;
    struct dirent dir_data;
    char path[PATH_MAX];
};

DIR *opendir(const char *dirname)
{
    struct __dirstream *ret = NULL;
    if (!dirname)
    {
        errno = EIO;
        return ret;
    }
    ret = calloc(1, sizeof(struct __dirstream));
    if (!ret)
    {
        errno = ENOMEM;
        return ret;
    }
    ret->position = 0;
    int err = vfs_opendir(&ret->dirh, dirname);
    if (err < 0)
    {
        free(ret);
        errno = -err;
        ret = NULL;
    }
    else
    {
        strncpy(ret->path, dirname, sizeof(ret->path));
    }
    return ret;
}

int closedir(DIR *dirp)
{
    if (!dirp)
    {
        errno = EBADF;
        return -1;
    }
    int ret = vfs_closedir(&dirp->dirh);
    if (ret < 0)
    {
        errno = -ret;
        ret = -1;
    }
    free(dirp);
    return ret;
}

struct dirent *readdir(DIR *dirp)
{
    if (!dirp)
    {
        errno = EBADF;
        return NULL;
    }
    int ret = vfs_readdir(&dirp->dirh, &dirp->dir_data);
    if (ret < 0)
    {
        errno = -ret;
        return NULL;
    }
    else
    {
        if (dirp->dir_data.d_name[0] == '\0')
        {
            return NULL;
        }
        else
        {
            return &dirp->dir_data;
        }
    }
}

int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result)
{
    if (!dirp)
    {
        errno = EBADF;
        return -1;
    }
    if (!*result)
    {
        errno = EINVAL;
        return -1;
    }
    int ret = vfs_readdir(&dirp->dirh, entry);
    if (ret < 0)
    {
        errno = -ret;
        return -1;
    }
    else
    {
        if (entry->d_name[0] == '\0')
        {
            *result = NULL;
            return 0;
        }
        else
        {
            *result = entry;
            return 0;
        }
    }
}

void rewinddir(DIR *dirp)
{
    if (!dirp)
    {
        errno = EBADF;
        return;
    }
    int err = vfs_closedir(&dirp->dirh);
    if (err < 0)
    {
        errno = -err;
        return;
    }
    err = vfs_opendir(&dirp->dirh, dirp->path);
    if (err < 0)
    {
        errno = -err;
        return;
    }
}

void seekdir(DIR *dirp, long int loc)
{
    if (!dirp)
    {
        errno = EBADF;
        return;
    }
    if (loc < 0)
    {
        errno = EINVAL;
        return;
    }
    if ((long)dirp->position > loc)
    {
        int err = vfs_closedir(&dirp->dirh);
        if (err < 0)
        {
            errno = -err;
            return;
        }
        err = vfs_opendir(&dirp->dirh, dirp->path);
        if (err < 0)
        {
            errno = -err;
            return;
        }
        dirp->position = 0;
    }
    while (((long)dirp->position < loc) && vfs_readdir(&dirp->dirh, &dirp->dir_data) == 0 && dirp->dir_data.d_name[0])
    {
        dirp->position += 1;
    }
}

long int telldir(DIR *dirp)
{
    if (!dirp)
    {
        errno = EBADF;
        return -1;
    }
    return dirp->position;
}
