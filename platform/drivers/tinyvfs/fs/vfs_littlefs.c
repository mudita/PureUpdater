#include <hal/tinyvfs.h>
#include <prv/tinyvfs/vfs_priv_data.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/statvfs.h>
#include <prv/tinyvfs/vfs_device.h>
#include <prv/tinyvfs/lfs_diskio.h>
#include <prv/tinyvfs/vfs_littlefs.h>
#include <sys/dirent.h>
#include <lfs.h>

//! Internal lfs configuration structure
struct dlfs_ctx {
    struct lfs_config cfg;      //! Lfs config
    lfs_t lfs;                  //! Lfs mount
};

static int lfs_to_errno(int error)
{
	if (error >= 0) {
		return error;
	}

	switch (error) {
	default:
	case LFS_ERR_IO:        /* Error during device operation */
		return -EIO;
	case LFS_ERR_CORRUPT:   /* Corrupted */
		return -EFAULT;
	case LFS_ERR_NOENT:     /* No directory entry */
		return -ENOENT;
	case LFS_ERR_EXIST:     /* Entry already exists */
		return -EEXIST;
	case LFS_ERR_NOTDIR:    /* Entry is not a dir */
		return -ENOTDIR;
	case LFS_ERR_ISDIR:     /* Entry is a dir */
		return -EISDIR;
	case LFS_ERR_NOTEMPTY:  /* Dir is not empty */
		return -ENOTEMPTY;
	case LFS_ERR_BADF:      /* Bad file number */
		return -EBADF;
	case LFS_ERR_FBIG:      /* File too large */
		return -EFBIG;
	case LFS_ERR_INVAL:     /* Invalid parameter */
		return -EINVAL;
	case LFS_ERR_NOSPC:     /* No space left on device */
		return -ENOSPC;
	case LFS_ERR_NOMEM:     /* No more memory available */
		return -ENOMEM;
	}
}

static int errno_to_lfs(int error)
{
	if (error >= 0) {
		return LFS_ERR_OK;
	}

	switch (error) {
	default:
	case -EIO:              /* Error during device operation */
		return LFS_ERR_IO;
	case -EFAULT:		/* Corrupted */
		return LFS_ERR_CORRUPT;
	case -ENOENT:           /* No directory entry */
		return LFS_ERR_NOENT;
	case -EEXIST:           /* Entry already exists */
		return LFS_ERR_EXIST;
	case -ENOTDIR:          /* Entry is not a dir */
		return LFS_ERR_NOTDIR;
	case -EISDIR:           /* Entry is a dir */
		return LFS_ERR_ISDIR;
	case -ENOTEMPTY:        /* Dir is not empty */
		return LFS_ERR_NOTEMPTY;
	case -EBADF:            /* Bad file number */
		return LFS_ERR_BADF;
	case -EFBIG:            /* File too large */
		return LFS_ERR_FBIG;
	case -EINVAL:           /* Invalid parameter */
		return LFS_ERR_INVAL;
	case -ENOSPC:           /* No space left on device */
		return LFS_ERR_NOSPC;
	case -ENOMEM:           /* No more memory available */
		return LFS_ERR_NOMEM;
	}
}

static int translate_flags(unsigned flags)
{
    int lfs_mode = 0;
    switch (flags & O_ACCMODE) {
    case O_RDONLY:
        lfs_mode |= LFS_O_RDONLY;
        break;
    case O_WRONLY:
        lfs_mode |= LFS_O_WRONLY;
        break;
    case O_RDWR:
        lfs_mode |= LFS_O_RDWR;
        break;
    }
    if (flags & O_APPEND) {
        lfs_mode |= LFS_O_APPEND;
    }
    if (flags & O_CREAT) {
        lfs_mode |= LFS_O_CREAT;
    }
    if (flags & O_TRUNC) {
        lfs_mode |= LFS_O_TRUNC;
    }
    if (flags & O_EXCL) {
        lfs_mode |= LFS_O_EXCL;
    }
    return lfs_mode;
}


static const char *strip_prefix(const char *path, const struct vfs_mount *mp)
{
	static const char *const root = "/";

	if ((path == NULL) || (mp == NULL)) {
		return path;
	}

	path += mp->mountp_len;
	return *path ? path : root;
}


static int dlfs_open(struct vfs_file *fp, const char *path, int flags, int mode)
{
	struct dlfs_ctx *fs = fp->mp->fs_data;
	struct lfs *lfs = &fs->lfs;
    fp->filep = calloc(1, sizeof(lfs_file_t));
    if(!fp->filep) {
        return -ENOMEM;
    }
	path = strip_prefix(path, fp->mp);
    int ret = lfs_file_open(&fs->lfs, fp->filep, path, translate_flags(flags));
	if (ret < 0) {
		free(fp->filep);
        fp->filep = NULL;
	}
	return lfs_to_errno(ret);
}


static int dlfs_close(struct vfs_file *fp)
{
	struct dlfs_ctx *fs = fp->mp->fs_data;
	int ret = lfs_file_close(&fs->lfs, fp->filep);
    free(fp->filep);
    fp->filep = NULL;
	return lfs_to_errno(ret);
}


static int dlfs_unlink(struct vfs_mount *mountp, const char *path)
{
	struct dlfs_ctx *fs = mountp->fs_data;
	path = strip_prefix(path, mountp);
	int ret = lfs_remove(&fs->lfs, path);
	return lfs_to_errno(ret);
}

static int dlfs_rename(struct vfs_mount *mountp, const char *from, const char *to)
{
	struct dlfs_ctx *fs = mountp->fs_data;
	from = strip_prefix(from, mountp);
	to = strip_prefix(to, mountp);
	int ret = lfs_rename(&fs->lfs, from, to);
	return lfs_to_errno(ret);
}

static ssize_t dlfs_read(struct vfs_file *fp, void *ptr, size_t len)
{
	struct dlfs_ctx *fs = fp->mp->fs_data;
	ssize_t ret = lfs_file_read(&fs->lfs, fp->filep, ptr, len);
	return lfs_to_errno(ret);
}

static ssize_t dlfs_write(struct vfs_file *fp, const void *ptr, size_t len)
{
	struct dlfs_ctx *fs = fp->mp->fs_data;
	ssize_t ret = lfs_file_write(&fs->lfs, fp->filep, ptr, len);
	return lfs_to_errno(ret);
}

static ssize_t dlfs_seek(struct vfs_file *fp, off_t off, int whence)
{
	struct dlfs_ctx *fs = fp->mp->fs_data;
	off_t ret = lfs_file_seek(&fs->lfs, fp->filep, off, whence);
	return lfs_to_errno(ret);
}

static off_t dlfs_tell(struct vfs_file *fp)
{
	struct dlfs_ctx *fs = fp->mp->fs_data;
	off_t ret = lfs_file_tell(&fs->lfs, fp->filep);
	return ret;
}

static int dlfs_sync(struct vfs_file *fp)
{
	struct dlfs_ctx *fs = fp->mp->fs_data;
	int ret = lfs_file_sync(&fs->lfs, fp->filep);
	return lfs_to_errno(ret);
}

static int dlfs_mkdir(struct vfs_mount *mountp, const char *path)
{
	struct dlfs_ctx *fs = mountp->fs_data;
	path = strip_prefix(path, mountp);
	int ret = lfs_mkdir(&fs->lfs, path);
	return lfs_to_errno(ret);
}


static int dlfs_opendir(struct vfs_dir *dp, const char *path)
{
	struct dlfs_ctx *fs = dp->mp->fs_data;
    dp->dirp = calloc(1, sizeof(lfs_dir_t));
    if(!dp->dirp) {
        return -ENOMEM;
    }
	path = strip_prefix(path, dp->mp);
	int ret = lfs_dir_open(&fs->lfs, dp->dirp, path);
	if (ret < 0) {
        free(dp->dirp);
        dp->dirp = NULL;
	}
	return lfs_to_errno(ret);
}

static void info_to_dirent(const struct lfs_info *info, struct dirent *entry)
{
	entry->d_type = ((info->type == LFS_TYPE_DIR) ?  DT_DIR : DT_REG);
	entry->d_size = info->size;
	strncpy(entry->d_name, info->name, sizeof(entry->d_name));
	entry->d_name[sizeof(entry->d_name) - 1] = '\0';
}

static int dlfs_readdir(struct vfs_dir *dp, struct dirent *entry)
{
	struct dlfs_ctx *fs = dp->mp->fs_data;
	struct lfs_info info;
	int ret = lfs_dir_read(&fs->lfs, dp->dirp, &info);
	if (ret > 0) {
		info_to_dirent(&info, entry);
		ret = 0;
	} else if (ret == 0) {
		entry->d_name[0] = 0;
	}
	return lfs_to_errno(ret);
}

static int dlfs_closedir(struct vfs_dir *dp)
{
	struct dlfs_ctx *fs = dp->mp->fs_data;
	int ret = lfs_dir_close(&fs->lfs, dp->dirp);
    free(dp->dirp);
    dp->dirp = NULL;
	return lfs_to_errno(ret);
}

static int attrib_to_st_mode(uint8_t type)
{
    mode_t mode = S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP | S_IWOTH;
    if (type == LFS_TYPE_REG) {
        mode |= S_IFREG;
    }
    else if (type == LFS_TYPE_DIR) {
        mode |= (S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH);
    }
    return mode;
}
static void info_to_stat(const struct lfs_config* cfg, const struct lfs_info* lfi,  struct stat* st)
{
    memset(st, 0, sizeof(*st));
    st->st_nlink   = 1;
    st->st_size   = lfi->size;
    st->st_blksize = cfg->block_size;
    st->st_blocks  = lfi->size / cfg->block_count;
    st->st_mode    = attrib_to_st_mode(lfi->type);
}


static int dlfs_stat(struct vfs_mount *mountp, const char *path, struct stat *entry)
{
	struct dlfs_ctx *fs = mountp->fs_data;
	path = strip_prefix(path, mountp);
	struct lfs_info info;
	int ret = lfs_stat(&fs->lfs, path, &info);
	if (ret >= 0) {
		info_to_stat(&fs->cfg, &info, entry);
		ret = 0;
	}
	return lfs_to_errno(ret);
}

static int dlfs_chmod(struct vfs_mount *mountp, const char*path, mode_t mode)
{
    struct stat st;
    return dlfs_stat(mountp,path,&st);
}


static int dlfs_truncate(struct vfs_file *fp, off_t length)
{
	struct dlfs_ctx *fs = fp->mp->fs_data;
	int ret = lfs_file_truncate(&fs->lfs, fp->filep, length);
	return lfs_to_errno(ret);
}

static int dlfs_statvfs(struct vfs_mount *mountp, const char *path, struct statvfs *stat)
{
	struct dlfs_ctx *fs = mountp->fs_data;
	struct lfs *lfs = &fs->lfs;
	stat->f_bsize = lfs->cfg->prog_size;
	stat->f_frsize = lfs->cfg->block_size;
	stat->f_blocks = lfs->cfg->block_count;
	path = strip_prefix(path, mountp);
	ssize_t ret = lfs_fs_size(lfs);
	if (ret >= 0) {
		stat->f_bfree = stat->f_blocks - ret;
		ret = 0;
	}
	return lfs_to_errno(ret);
}


static int dlfs_mount(struct vfs_mount *mountp)
{
    mountp->fs_data = calloc(1 , sizeof(struct dlfs_ctx));
    if(!mountp->fs_data) {
        return -ENOMEM;
    }
	struct dlfs_ctx *fs = mountp->fs_data;
    int ret = vfs_lfs_append_volume(mountp->storage_dev, &fs->cfg);
    if( ret ) {
        free(mountp->fs_data);
        mountp->fs_data = NULL;
        return ret;
    }
    ret = lfs_mount(&fs->lfs, &fs->cfg);
    if( ret ) {
        free(mountp->fs_data);
        mountp->fs_data = NULL;
        vfs_lfs_remove_volume(&fs->cfg);
    }
    return lfs_to_errno(ret);
}

static int dlfs_unmount(struct vfs_mount *mountp)
{
	struct dlfs_ctx *fs = mountp->fs_data;
    int ret = lfs_unmount( &fs->lfs );
    free(mountp->fs_data);
    mountp->fs_data = NULL;
    return lfs_to_errno(ret);
}


// Littlefs filesystem operations private structure
static const struct vfs_filesystem_ops lfs_fops = 
{
    .open = dlfs_open,
    .read = dlfs_read,
    .write = dlfs_write,
    .lseek = dlfs_seek,
    .tell = dlfs_tell,
    .truncate = dlfs_truncate,
    .sync = dlfs_sync,
    .close = dlfs_close,
    .opendir = dlfs_opendir,
    .readdir = dlfs_readdir,
    .closedir = dlfs_closedir,
    .mount = dlfs_mount,
    .unmount = dlfs_unmount,
    .unlink = dlfs_unlink,
    .rename = dlfs_rename,
    .mkdir = dlfs_mkdir,
    .stat = dlfs_stat,
    .statvfs = dlfs_statvfs,
    .chmod = dlfs_chmod
};

/** Enable littlefs filesystem
 * @return error code
 */
int vfs_priv_enable_littlefs_filesystem(void)
{
    return vfs_register_filesystem(vfs_fs_littlefs, &lfs_fops);
}