#pragma once
#include <stdint.h>
#include <stdbool.h>


#define BLKDEV_DISK_SHIFT 8             //! Blkdev disk shift
#define BLKDEV_DISK_MASK 0xffff         //! Blkdev disk mask
#define BLKDEV_PART_SHIFT 0             //! Blkdev part shift
#define BLKDEV_PART_MASK 0xff           //! Blkdev part mask


//! Return hardware disk device
static inline int blk_hwdisk(int device)
{
    return (device >> BLKDEV_DISK_SHIFT) & BLKDEV_DISK_MASK;
}

//! Return the hardware partition
static inline int blk_hwpart(int device)
{
    return (device >> BLKDEV_PART_SHIFT) & BLKDEV_PART_MASK;
}