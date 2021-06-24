#include <hal/blk_dev.h>
#include <ff.h>
#include <ff_diskio.h>
#include <stdio.h>
#include <errno.h>


/** We have currently simplified model of the drive mapping
 * Currently drive 1 -9 translates directly 
 * to the partition number on the EMMC disk
 * for example drive 1: is mapped to the first partition
 * drive 2: is mapped to the second partition etc.
 */

static inline int pdrive_to_blk( BYTE pdrv )
{
    return blk_disk_handle( blkdev_emmc_user, pdrv );
}


DSTATUS disk_initialize(BYTE pdrv)
{
    const int disk = pdrive_to_blk(pdrv);
    blk_partition_t* parts;
    const int nparts = blk_get_partitions(disk,&parts);
    if(nparts<0) {
        printf("vfat: Unable to get drive partition list error: %i\n", nparts);
        return STA_NOINIT;
    }
    if(nparts<pdrv) {
        printf("vfat: Partition out of range. %i paritions exists\n", nparts);
        return STA_NODISK;
    }
    const blk_partition_t* part = &parts[pdrv];
    if( part->type !=  blk_part_type_vfat ) {
        printf("vfat: Non vfat partition type code %i\n", part->type);
        return STA_NODISK;
    }
    return RES_OK;
}

DSTATUS disk_status(BYTE pdrv)
{
    return disk_initialize(pdrv);
}

DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count)
{
    const int disk = pdrive_to_blk(pdrv);
    const int err = blk_read( disk, sector, count, buff );
    if( err < 0 ) {
        printf("vfat: Unable to read to the disc errno %i\n", err);
        return (err == -ERANGE) ? (RES_PARERR) : (RES_ERROR);
    }
    return RES_OK;
}

DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count)
{ 
    const int disk = pdrive_to_blk(pdrv);
    const int err = blk_write( disk, sector, count, buff );
    if( err < 0 ) {
        printf("vfat: Unable to write to the disc errno %i\n", err);
        return (err == -ERANGE) ? (RES_PARERR) : (RES_ERROR);
    }
    return RES_OK;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
    DRESULT res;
    const int disk = pdrive_to_blk(pdrv);
    int err;
    blk_dev_info_t dinfo;
    switch(cmd) {
        case CTRL_SYNC:
            res = RES_OK;
            break;
        case GET_SECTOR_COUNT:
        case GET_SECTOR_SIZE:
        case GET_BLOCK_SIZE:
            err = blk_info(disk, &dinfo);
            if(!err) {
                switch(cmd) {
                    case GET_SECTOR_COUNT:
                        *(uint32_t*)buff = dinfo.sector_count; break;
                    case GET_SECTOR_SIZE:
                        *(uint32_t*)buff = dinfo.sector_size; break;
                    case GET_BLOCK_SIZE:
                        *(uint32_t*)buff = dinfo.erase_group; break;
                }
                res = RES_OK;
            } else {
                printf("vfat: ioctl error blkinfo %i\n", err );
                res = RES_PARERR;
            }
            break;
        default:
            printf("vfat: ioctl unimplemented %i\n", cmd);
            res = RES_PARERR;
            break;
        }
    return res;
}