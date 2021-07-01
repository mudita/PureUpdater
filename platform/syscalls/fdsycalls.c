#include <errno.h>
#include <hal/console.h>
#include <hal/tinyvfs.h>
#include <stdlib.h>

#define MAX_OPEN_FILES 256
#define FIRST_HANDLE 3

//! Static array for holding VFS file handles
static struct vfs_file *file_handles[MAX_OPEN_FILES];

int _open(const char *file, int flags, int mode)
{
    int hwid = -1;
    for (size_t n = 0; n < MAX_OPEN_FILES; ++n)
    {
        if (file_handles[n] == NULL)
        {
            hwid = n;
            break;
        }
    }
    if (hwid < 0)
    {
        errno = EMFILE;
        return hwid;
    }
    file_handles[hwid] = calloc(1, sizeof(struct vfs_file));
    struct vfs_file *fil = file_handles[hwid];
    if (!fil)
    {
        errno = ENOMEM;
        return -1;
    }
    const int err = vfs_open(fil, file, flags, mode);
    if (err < 0)
    {
        free(fil);
        fil = NULL;
        errno = -err;
        return -1;
    }
    return hwid + FIRST_HANDLE;
}

// Write syscalls
_ssize_t _write(int fd, const char *buf, size_t nbyte)
{
    // STDOUT and STDERR are routed to the trace device
    if (fd == 1 || fd == 2)
    {
        int error = debug_console_write(buf, nbyte);
        if (error < 0)
        {
            errno = -error;
            return -1;
        }
        else
        {
            return error;
        }
    }
    else
    {
        fd -= FIRST_HANDLE;
        if (fd >= MAX_OPEN_FILES || fd < 0)
        {
            errno = EBADF;
            return -1;
        }
        struct vfs_file *fil = file_handles[fd];
        if (!fil)
        {
            errno = EBADF;
            return -1;
        }
        const int err = vfs_write(fil, buf, nbyte);
        if (err < 0)
        {
            errno = -err;
            return -1;
        }
        return err;
    }
}

// Get VFS handle or set errno on failure
static struct vfs_file *to_vfs_handle(int fd)
{
    fd -= FIRST_HANDLE;
    if (fd >= MAX_OPEN_FILES || fd < 0)
    {
        errno = EBADF;
        return NULL;
    }
    struct vfs_file *fil = file_handles[fd];
    if (!fil)
    {
        errno = EBADF;
    }
    return fil;
}

//! Read syscalls
int _read(int fd, char *ptr, size_t len)
{
    if (fd == 0)
    {
        errno = ENOTSUP;
        return -1;
    }
    else
    {
        struct vfs_file *fil = to_vfs_handle(fd);
        if (!fil)
        {
            return -1;
        }
        const int err = vfs_read(fil, ptr, len);
        if (err < 0)
        {
            errno = -err;
            return -1;
        }
        return err;
    }
}

int _close(int fd)
{
    struct vfs_file *fil = to_vfs_handle(fd);
    if (!fil)
    {
        return -1;
    }
    int err = vfs_close(fil);
    if (err < 0)
    {
        errno = -err;
        err = -1;
    }
    free(fil);
    file_handles[fd - FIRST_HANDLE] = NULL;
    return err;
}

int _lseek(int fd, int pos, int dir)
{
    struct vfs_file *fil = to_vfs_handle(fd);
    if (!fil)
    {
        return -1;
    }
    int err = vfs_seek(fil, pos, dir);
    if (err < 0)
    {
        errno = -err;
        err = -1;
    }
    return err;
}

int fsync(int fd)
{
    struct vfs_file *fil = to_vfs_handle(fd);
    if (!fil)
    {
        return -1;
    }
    int err = vfs_sync(fil);
    if (err < 0)
    {
        errno = -err;
        err = -1;
    }
    return err;
}

int ftruncate(int fd, off_t len)
{
    struct vfs_file *fil = to_vfs_handle(fd);
    if (!fil)
    {
        return -1;
    }
    int err = vfs_ftruncate(fil, len);
    if (err < 0)
    {
        errno = -err;
        err = -1;
    }
    return err;
}