#include <hal/tinyvfs.h>
#include <prv/tinyvfs/vfs_priv_data.h>
#include <ff.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/statvfs.h>
#include <prv/tinyvfs/vfs_device.h>
#define DIRENT_NO_DIR_STRUCTURE 1
#include <dirent.h>

static int translate_error(int error)
{
	switch (error) {
	case FR_OK:
		return 0;
	case FR_NO_FILE:
	case FR_NO_PATH:
	case FR_INVALID_NAME:
		return -ENOENT;
	case FR_DENIED:
		return -EACCES;
	case FR_EXIST:
		return -EEXIST;
	case FR_INVALID_OBJECT:
		return -EBADF;
	case FR_WRITE_PROTECTED:
		return -EROFS;
	case FR_INVALID_DRIVE:
	case FR_NOT_ENABLED:
	case FR_NO_FILESYSTEM:
		return -ENODEV;
	case FR_NOT_ENOUGH_CORE:
		return -ENOMEM;
	case FR_TOO_MANY_OPEN_FILES:
		return -EMFILE;
	case FR_INVALID_PARAMETER:
		return -EINVAL;
	case FR_LOCKED:
	case FR_TIMEOUT:
	case FR_MKFS_ABORTED:
	case FR_DISK_ERR:
	case FR_INT_ERR:
	case FR_NOT_READY:
		return -EIO;
	}

	return -EIO;
}


static uint8_t translate_flags(unsigned flags)
{
    uint8_t fat_mode = 0;
    switch (flags & O_ACCMODE)
    {
    case O_RDONLY:
        fat_mode |= FA_READ;
        break;
    case O_WRONLY:
        fat_mode |= FA_WRITE;
        break;
    case O_RDWR:
        fat_mode |= (FA_READ | FA_WRITE);
        break;
    }
    if (flags & O_APPEND)
        fat_mode |= FA_OPEN_APPEND;
    if (flags & O_CREAT)
        fat_mode |= FA_OPEN_ALWAYS;
    if (flags & O_TRUNC)
        fat_mode |= FA_CREATE_ALWAYS;
    return fat_mode;
}

static mode_t translate_fat_attrib_to_mode(BYTE fattrib)
{
	mode_t mode = S_IRUSR | S_IRGRP | S_IROTH;
	if (fattrib & AM_DIR)
	{
		mode |= (S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH);
	}
	else
	{
		mode |= S_IFREG;
	}
	if ((fattrib & AM_RDO) == 0)
	{
		mode |= (S_IWUSR | S_IWGRP | S_IWOTH);
	}
	if (fattrib & AM_HID)
	{
	}
	if (fattrib & AM_SYS)
	{
	}
	if (fattrib & AM_ARC)
	{
	}
	return mode;
}

static void translate_filinfo_to_stat(const FILINFO *fs,struct stat *st)
{
	st->st_dev = 0;
	st->st_ino = 0;
	st->st_mode  = translate_fat_attrib_to_mode(fs->fattrib);
	st->st_nlink = 1;
	st->st_uid   = 0;
	st->st_gid   = 0;
	st->st_rdev  = 0;
	st->st_size  = fs->fsize;
	// TODO: Block FF_MIN_SS != FF_MAX_SS
#if FF_MAX_SS != FF_MIN_SS
	st->st_blksize = fatfs->ssize;
#else
	st->st_blksize = FF_MIN_SS;
#endif
	// TODO: Time is currently not supported
	st->st_blocks = fs->fsize / st->st_blksize;
	st->st_atime  = 0;
	st->st_mtime  = 0;
	st->st_ctime  = 0;
}

static int ffat_open(struct vfs_file *filp, const char *file_name, int flags, int mode)
{
	FRESULT res;
	uint8_t fs_mode;
	void *ptr;

	if ((ptr=calloc(1,sizeof(struct vfs_file)))) {
		filp->filep = ptr;
	} else {
		return -ENOMEM;
	}
	fs_mode = translate_flags(mode);
	res = f_open(filp->filep, &file_name[1], fs_mode);
	if (res != FR_OK) {
		free(ptr);
		filp->filep = NULL;
	}
	return translate_error(res);
}

static BYTE translate_mode_to_attrib(mode_t mode)
{
	BYTE attr = 0;
	if ((mode & (S_IWGRP | S_IWUSR | S_IWOTH)) == 0)
		attr |= AM_RDO;
	return attr;
}

static int ffat_close(struct vfs_file *filp)
{
	FRESULT res;
	res = f_close(filp->filep);
	/* Free file ptr memory */
	free(filp->filep);
	filp->filep = NULL;
	return translate_error(res);
}

static int ffat_unlink(struct vfs_mount *mountp, const char *path)
{
	FRESULT res;
	res = f_unlink(&path[1]);
	return translate_error(res);
}

static int ffat_chmod(struct vfs_mount* mountp, const char *path, mode_t mode)
{
	FRESULT res;
	res = f_chmod(&path[1], translate_mode_to_attrib(mode), translate_mode_to_attrib(mode));
	return translate_error(res);
}

static int ffat_rename(struct vfs_mount *mountp, const char *from, const char *to)
{
	FRESULT err;
	FILINFO fno;

	/* Check if 'to' path exists; remove it if it does */
	err = f_stat(&to[1], &fno);
	if (err == FR_OK) {
		err = f_unlink(&to[1]);
		if (err!=FR_OK)
			return translate_error(err);
	}

	err = f_rename(&from[1], &to[1]);
	return translate_error(err);
}

static ssize_t ffat_read(struct vfs_file *filp, void *ptr, size_t size)
{
	FRESULT res;
	unsigned int br;

	res = f_read(filp->filep, ptr, size, &br);
	if (res != FR_OK) {
		return translate_error(res);
	}

	return br;
}

static ssize_t ffat_write(struct vfs_file *filp, const void *ptr, size_t size)
{
	FRESULT res = FR_OK;
	unsigned int bw;
	off_t pos = f_size((FIL *)filp->filep);

	if (filp->flags & O_APPEND) {
		res = f_lseek(filp->filep, pos);
	}

	if (res == FR_OK) {
		res = f_write(filp->filep, ptr, size, &bw);
	}

	if (res != FR_OK) {
		return translate_error(res);
	}

	return bw;
}

static int ffat_seek(struct vfs_file *filp, off_t offset, int whence)
{
	FRESULT res = FR_OK;
	off_t pos;

	switch (whence) {
	case SEEK_SET:
		pos = offset;
		break;
	case SEEK_CUR:
		pos = f_tell((FIL *)filp->filep) + offset;
		break;
	case SEEK_END:
		pos = f_size((FIL *)filp->filep) + offset;
		break;
	default:
		return -EINVAL;
	}

	if ((pos < 0) || (pos > (off_t)f_size((FIL *)filp->filep))) {
		return -EINVAL;
	}

	res = f_lseek(filp->filep, pos);

	return translate_error(res);
}

static off_t ffat_tell(struct vfs_file *filp)
{
	return f_tell((FIL *)filp->filep);
}


static int ffat_truncate(struct vfs_file *filp, off_t length)
{
	FRESULT res = FR_OK;
	off_t cur_length = f_size((FIL *)filp->filep);
	res = f_lseek(filp->filep, length);
	if (res != FR_OK) {
		return translate_error(res);
	}
	if (length < cur_length) {
		res = f_truncate(filp->filep);
	} else {
		length = f_tell((FIL *)filp->filep);

		res = f_lseek(filp->filep, cur_length);
		if (res != FR_OK) {
			return translate_error(res);
		}
		unsigned int bw;
		uint8_t c = 0U;

		for (int i = cur_length; i < length; i++) {
			res = f_write(filp->filep, &c, 1, &bw);
			if (res != FR_OK) {
				break;
			}
		}
	}
	return translate_error(res);
}

static int ffat_sync(struct vfs_file *filp)
{
	FRESULT res = FR_OK;
	res = f_sync(filp->filep);
	return translate_error(res);
}

static int ffat_mkdir(struct vfs_mount *mountp, const char *path)
{
	FRESULT res;
	res = f_mkdir(&path[1]);
	return translate_error(res);
}

static int ffat_opendir(struct vfs_dir *dirp, const char *path)
{
	FRESULT res;
	void *ptr;
	if ((ptr=calloc(1,sizeof(struct vfs_dir)))) {
		dirp->dirp = ptr;
	} else {
		return -ENOMEM;
	}
	res = f_opendir(dirp->dirp, &path[1]);
	if (res != FR_OK) {
		free(ptr);
		dirp->next_mnt = NULL;
	}
	return translate_error(res);
}


static int ffat_readdir(struct vfs_dir *dirp, struct dirent *entry)
{
	FRESULT res;
	FILINFO fno;

	res = f_readdir(dirp->dirp, &fno);
	if (res == FR_OK) {
		strcpy(entry->d_name, fno.fname);
		if (entry->d_name[0] != 0) {
			entry->d_type = (fno.fattrib & AM_DIR) ?  DT_DIR : DT_REG;
			entry->d_size = fno.fsize;
		}
	}

	return translate_error(res);
}

static int ffat_closedir(struct vfs_dir *zdp)
{
	FRESULT res;
	res = f_closedir(zdp->dirp);
	/* Free file ptr memory */
	free(zdp->dirp);
	zdp->dirp = NULL;
	return translate_error(res);
}

static int ffat_stat(struct vfs_mount *mountp, const char *path, struct stat *entry)
{
	FRESULT res;
	FILINFO fno;
	res = f_stat(&path[1], &fno);
	if (res == FR_OK) {
		translate_filinfo_to_stat(&fno, entry);
	}
	return translate_error(res);
}


static int ffat_statvfs(struct vfs_mount *mountp, const char *path, struct statvfs *stat)
{
	FATFS *fs;
	FRESULT res;
	res = f_getfree(&mountp->mnt_point[1], &stat->f_bfree, &fs);
	if (res != FR_OK) {
		return -EIO;
	}
	stat->f_bsize = FF_MIN_SS;
	stat->f_frsize = fs->csize * stat->f_bsize;
	stat->f_blocks = (fs->n_fatent - 2);
	return translate_error(res);
}


static int ffat_mount(struct vfs_mount *mountp)
{
	FRESULT res;
	res = f_mount(mountp->fs_data, &mountp->mnt_point[1], 1);
	return translate_error(res);

}

static int ffat_unmount(struct vfs_mount *mountp)
{
	FRESULT res;
	res = f_mount(NULL, &mountp->mnt_point[1], 1);
	return translate_error(res);
}


// VFAT fileystem operations private structure
static const struct vfs_filesystem_ops vfat_fops = 
{
	.open = ffat_open,
	.read = ffat_read,
	.write = ffat_write,
	.lseek = ffat_seek,
	.tell = ffat_tell,
	.truncate = ffat_truncate,
	.sync = ffat_sync,
	.close = ffat_close,
	.opendir = ffat_opendir,
	.readdir = ffat_readdir,
	.closedir = ffat_closedir,
	.mount = ffat_mount,
	.unmount = ffat_unmount,
	.unlink = ffat_unlink,
	.rename = ffat_rename,
	.mkdir = ffat_mkdir,
	.stat = ffat_stat,
	.statvfs = ffat_statvfs,
	.chmod = ffat_chmod
};


/** Enable vfat filesystem
 * @return error code
 */
int vfs_priv_enable_vfat_fileystem(void) 
{
	return vfs_register_filesystem(vfs_fs_fat, &vfat_fops);
}
