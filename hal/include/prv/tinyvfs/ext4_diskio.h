// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

struct ext4_blockdev;

/** Append volume to the selected partition block
 * @param[in] blkdev block device for append
 * @param[out] bdev Ext4 block device objecvt
 * @return Error code on success otherwise error
 */
int vfs_ext4_append_volume(int blkdev, struct ext4_blockdev **bdev);

/** Remove volume from the LFS and destroy
 * the context
 * @param lfsc Lfs configuration structure
 */
void vfs_ext4_remove_volume(struct ext4_blockdev *lfsc);
