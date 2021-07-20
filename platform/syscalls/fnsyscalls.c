#include <errno.h>
#include <hal/tinyvfs.h>
#include <stdlib.h>
#include <reent.h>

int mkdir(const char *path, mode_t mode)
{
    (void)mode;
    int err = vfs_mkdir(path);
    if (err < 0)
    {
        errno = -err;
        err = -1;
    }
    return err;
}

int _unlink(char *name)
{
    int err = vfs_unlink(name);
    if (err < 0)
    {
        errno = -err;
        err = -1;
    }
    return err;
}

int _rename_r(struct _reent *reent, const char *oldpath, const char *newpath)
{
    int err = vfs_rename(oldpath, newpath);
    if (err < 0)
    {
        reent->_errno = -err;
        err = -1;
    }
    return err;
}

int _stat(const char *file, struct stat *st)
{
    int err = vfs_stat(file, st);
    if (err < 0)
    {
        errno = -err;
        err = -1;
    }
    return err;
}

int chmod(const char *path, mode_t mode)
{
    int err = vfs_chmod(path, mode);
    if (err < 0)
    {
        errno = -err;
        err = -1;
    }
    return err;
}

int statvfs(const char *path, struct statvfs *buf)
{
    int err = vfs_statvfs(path, buf);
    if (err < 0)
    {
        errno = -err;
        err = -1;
    }
    return err;
}
int truncate(const char *path, off_t length)
{
    int err = vfs_truncate(path, length);
    if (err < 0)
    {
        errno = -err;
        err = -1;
    }
    return err;
}
