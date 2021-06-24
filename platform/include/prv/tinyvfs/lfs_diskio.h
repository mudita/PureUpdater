/** Glue header with the LFS and the device driver
 */
#pragma once


/** Append volume to the selected partition block
 * @param[in] blkdev block device for append 
 * @param[out] lfsc Block configuration structure
 * @return Error code on success otherwise error
 */
int vfs_lfs_append_volume( int blkdev, struct lfs_config* lfsc );

/** Remove volume from the LFS and destroy 
 * the context
 * @param lfsc Lfs configuration structure
 */
void vfs_lfs_remove_volume(struct lfs_config *lfsc);