#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

//! Current devices
enum blkdev_devices {
    blkdev_emmc_user,   //! Emmc disk drive user partition
    blkdev_emmc_boot1,  //! Emmc disk boot0 partition
    _blkdev_eot_        //! Last item
};


//! Logical block address devic
typedef uint32_t lba_t;
//! LBA block size
typedef uint32_t blk_size_t;

//! Partition type enumeration
enum blk_part_type {
    blk_part_type_hardware = 0xffff    //! Hardware partition
};
//! Block partition structure
typedef struct blk_partition {
    int logical_num;            //! Logical number in the disk manager
    int mbr_num;                //! If mbr partition number
    lba_t start_sector;         //! First sector
    blk_size_t num_sectors;     //! Number of sectors
    bool bootable;              //! Partition is bootable
    unsigned char boot_unit;    //! 7 bit boot unit field
    unsigned short type;        //! Partition code
    size_t erase_blk;           //! Extra erase block if present
} blk_partition_t;

//! MBR partition codes
enum blk_part_types {
    blk_part_type_vfat = 0x0b,  //! VFAT partition
    blk_part_type_lfs  = 0x9e   //! Little fs partition
};

/** Retrive disk handle based on the disk and partition
 * @param hwdisk hardware disk identifier
 * @param partition partition number. 0 raw disc
 */
int blk_disk_handle(uint16_t hwdisk, uint8_t partition);


/**  Initialize block device driver
 * @return 0 otherwise error from ERRNO
 */
int blk_initialize(void);

/** Parse partitions on the device
 * @param device Major device number
 * @param parts Pointer to the block partition structures
 * @return if positive number of partitions negative error code
 */
int blk_get_partitions(int device, blk_partition_t** parts);

/** Get single partition on the devic
 * @param device Major device number
 * @param parts Pointer to the block partition structures
 * @return zero on success otherwise error
 * @note device must be a partition handle not a raw disc
 */
int blk_get_partition(int device, blk_partition_t* part);

/** Read block device data 
 * @param device Device identifier
 * @param lba Logical block address
 * @param lba_count Number of LBA sectors count
 * @param buf Buffer for data reead
 * @return 0 otherwise errno when error
 */
int blk_read(int device, lba_t lba, blk_size_t lba_count, void *buf );

/** Read block device data 
 * @param device Device identifier
 * @param lba Logical block address
 * @param lba_count Number of LBA sectors count
 * @param buf Buffer for data reead
 * @return 0 otherwise errno when error
 */
int blk_write( int device, lba_t lba, blk_size_t lba_count, const void *buf );

//! Block information type returned 
typedef struct blk_dev_info
{
    blk_size_t sector_size;     //! Sector size
    blk_size_t sector_count;    //! Sector count
    blk_size_t erase_group;     //! Erase group blocks
} blk_dev_info_t;

/** Get block device info
 * get current block device info
 * @param device Input device id
 * @param info Info structore
 * @param 0 otherwise error code
 */
int blk_info( int device, blk_dev_info_t* info );
