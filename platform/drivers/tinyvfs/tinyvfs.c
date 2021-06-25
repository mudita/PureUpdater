#include <prv/tinyvfs/vfs_device.h>
#include <prv/tinyvfs/vfs_priv_data.h>
#include <hal/tinyvfs.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>


struct vfs_filesystem_entry
{
    const struct vfs_filesystem_ops* ops;
    vfs_filesystem_type_t type; 
};

struct vfs_mount_entry
{

    struct vfs_mount* mnt;              // List of mount points
    struct vfs_mount_entry* next;       // Next mount entry
};

struct vfs_context 
{
    struct vfs_mount_entry* fopsl;        // FIlesystem entry list
    struct vfs_filesystem_entry* fste;    // Filesystem entry
    size_t num_fse;                       // Filesystem ops
};

// VFS context state
static struct vfs_context ctx;


static const struct vfs_filesystem_ops *fs_type_get(int type)
{
	for (size_t i = 0; i < ctx.num_fse; ++i) {
        if( ctx.fste[i].type == type ) {
            return ctx.fste[i].ops;
        }
	}
	return NULL;
}




static int fs_get_mnt_point(struct vfs_mount **mnt_pntp, const char *name, size_t *match_len)
{
	struct vfs_mount *mnt_p = NULL, *itr;
	size_t longest_match = 0;
	size_t len, name_len = strlen(name);
    for( struct vfs_mount_entry *e=ctx.fopsl; e; e=e->next )
    {
        struct vfs_mount* itr = e->mnt;
		len = itr->mountp_len;

		/*
		 * Move to next node if mount point length is
		 * shorter than longest_match match or if path
		 * name is shorter than the mount point name.
		 */
		if ((len < longest_match) || (len > name_len)) {
			continue;
		}

		/*
		 * Move to next node if name does not have a directory
		 * separator where mount point name ends.
		 */
		if ((len > 1) && (name[len] != '/') && (name[len] != '\0')) {
			continue;
		}

		/* Check for mount point match */
		if (strncmp(name, itr->mnt_point, len) == 0) {
			mnt_p = itr;
			longest_match = len;
		}
	}

	if (mnt_p == NULL) {
		return -ENOENT;
	}

	*mnt_pntp = mnt_p;
	if (match_len)
		*match_len = mnt_p->mountp_len;

	return 0;
}

/** 
 * Register the filesystem
 * @param type Filesystem type
 * @param fops File system operation structure
 * @return error otherwvise success
 */
int vfs_register_filesystem(int type, const struct vfs_filesystem_ops *fops)
{
	int err = 0;
	if (fs_type_get(type) != NULL) {
		err = -EEXIST;
	} else {
        ctx.fste = reallocarray(ctx.fste, ctx.num_fse + 1, sizeof(struct vfs_filesystem_entry));
        if (!ctx.fste)
        {
            printf("vfs: %s: No more memory available\n", __PRETTY_FUNCTION__);
            return -ENOMEM;
        }
        ctx.fste[ctx.num_fse++] = (struct vfs_filesystem_entry){.ops = fops, .type = type};
    }
	return err;
}


int vfs_mount(struct vfs_mount *mp)
{
	struct fs_mount_t *itr;
	const struct vfs_filesystem_ops *fs;
	size_t len = 0;

	/* API sanity check 
	 */
	if ((mp == NULL) || (mp->mnt_point == NULL)) {
		printf("vfs: %s mount point not initialized\n", __PRETTY_FUNCTION__);
		return -EINVAL;
	}

	len = strlen(mp->mnt_point);

	if ((len <= 1) || (mp->mnt_point[0] != '/')) {
		printf("vfs: %s invalid mount point\n", __PRETTY_FUNCTION__);
		return -EINVAL;
	}

	/* Check if mount point already exists */
    for( struct vfs_mount_entry *e=ctx.fopsl; e; e=e->next )
    {
        struct vfs_mount* itr = e->mnt;
		/* continue if length does not match */
		if (len != itr->mountp_len) {
			continue;
		}

		if (strncmp(mp->mnt_point, itr->mnt_point, len) == 0) {
            printf("vfs: %s mount point already exists\n",__PRETTY_FUNCTION__);
			return -EBUSY;
		}
	}

	/* Get file system information */
	fs = fs_type_get(mp->type);
	if (fs == NULL) {
        printf("vfs: %s Filesystem not registered\n",__PRETTY_FUNCTION__);
		return -ENOENT;
	}

	if (fs->mount == NULL) {
        printf("vfs: %s Filesystem %i doesn't support mount\n",__PRETTY_FUNCTION__, mp->type);
		return  -ENOTSUP;
	}

	if (fs->unmount == NULL) {
        printf("vfs warn: %s Filesystem %i doesn't support unmount\n",__PRETTY_FUNCTION__, mp->type);
	}

	int err = fs->mount(mp);
	if (err < 0) {
        printf("vfs warn: %s Filesystem mount error (%i)\n",__PRETTY_FUNCTION__, err);
        return err;
	}
	/* Update mount point data and append it to the list */
	mp->mountp_len = len;
	mp->fs = fs;
    {
        struct vfs_mount_entry *curr = ctx.fopsl;
        while(curr->next) curr = curr->next;
        curr->next = calloc(1, sizeof(struct vfs_mount_entry));
        curr->mnt = mp;
    }
	return err;
}

int vfs_unmount(struct vfs_mount *mp)
{
	int err = -EINVAL;

	if (mp == NULL) {
		return err;
	}

	if (mp->fs == NULL) {
		printf("vfs: %s Filesystem not mounted %p\n", __PRETTY_FUNCTION__, mp);
        return -EINVAL;
	}

	if (mp->fs->unmount == NULL) {
		printf("vfs: %s Umount is not supported\n", __PRETTY_FUNCTION__);
		return -ENOTSUP;
	}

	err = mp->fs->unmount(mp);
	if (err < 0) {
		printf("vfs: %s Umount error %i\n", __PRETTY_FUNCTION__, err);
        return -EINVAL;
	}

	/* clear file system interface */
	mp->fs = NULL;

	/* remove mount node from the list */
    for( struct vfs_mount_entry *c=ctx.fopsl; c; c=c->next )
    {
        if( c->mnt == mp ) {
            struct vfs_mount_entry* n = c->next;
            free(c);
            c = n;
        }
    }
    return err;
}


/* File operations */
int vfs_open(struct vfs_file *zfp, const char *file_name, int flags, mode_t mode)
{
	struct vfs_mount *mp;

	/* COpy flags to zfp for use with other fs_ API calls */
	zfp->flags = flags;
    zfp->mode = mode;

	if ((file_name == NULL) ||
			(strlen(file_name) <= 1) || (file_name[0] != '/')) {
		printf("vfs: %s Invalid file name\n", __PRETTY_FUNCTION__);
		return -EINVAL;
	}

	int err = fs_get_mnt_point(&mp, file_name, NULL);
	if (err < 0) {
		printf("vfs: %s Mount point not found\n", __PRETTY_FUNCTION__);
		return err;
	}

	zfp->mp = mp;

	if (zfp->mp->fs->open != NULL) {
		err = zfp->mp->fs->open(zfp, file_name, flags, mode);
		if (err < 0) {
            printf("vfs: %s File open error %i\n", __PRETTY_FUNCTION__, err);
			return err;
		}
	} else {
        return -EINVAL;
    }
    return err;
}

int fs_close(struct vfs_file *zfp)
{
	int err = -EINVAL;
	if (zfp->mp == NULL) {
		return 0;
	}
	if (zfp->mp->fs->close != NULL) {
		err = zfp->mp->fs->close(zfp);
		if (err < 0) {
            printf("vfs: %s File close error %i\n", __PRETTY_FUNCTION__, err);
			return err;
		}
	}
	zfp->mp = NULL;
	return err;
}