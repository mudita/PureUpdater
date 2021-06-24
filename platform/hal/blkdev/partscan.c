#include <prv/blkdev/partscan.h>
#include <hal/blk_dev.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


//! MBR partition constants
static const size_t mbr_signature_offs = 0x1FE;
static const size_t mbr_ptbl_offs      = 0x1BE;
static const size_t mbr_ptbl_active    = 0x000;
static const size_t mbr_ptbl_type      = 0x004;
static const size_t mbr_ptbl_sect_cnt  = 0x00c;
static const size_t mbr_ptbl_lba       = 0x008;
static const size_t ptbl_offs          = 0x1be;
static const size_t ptbl_size          = 16;
static const size_t ext_part           = 0x05;
static const size_t ext_linux_part     = 0x85;
static const size_t ext_win98_part     = 0x0f;
static const size_t reserved_sect      = 0x00e;
static const size_t number_of_fats     = 0x010;
static const size_t num_parts          = 4;
static const size_t min_sector_size    = 512;

static const size_t EXT_MAX_NUM_PARTS = 100;
static const size_t MAX_NUM_PARTS = EXT_MAX_NUM_PARTS + 4;
static const size_t MBR_ERASE_BLK_OFFSET = 0x00E0;

struct partitions {
    blk_partition_t *part;
    size_t nparts;
};

// ! For cleanup dynamic resources
static void free_clean_up(uint8_t** ptr) {
  free(*ptr);
}


// TO word
static inline uint32_t to_word(const uint8_t vec[], size_t offs)
{
    const uint8_t* buf = &vec[offs];
    return ((uint32_t)buf[0] << 0U) | ((uint32_t)buf[1] << 8U) | 
    ((uint32_t)buf[2] << 16U) | ((uint32_t)buf[3] << 24U);
}

// TO short
static inline uint16_t to_short(const uint8_t vec[], size_t offs)
{
    const uint8_t* buf = &vec[offs];
    return ((uint16_t)buf[0] << 0U) | ((uint16_t)buf[1] << 8U);
}

//* Read partitions info
static void read_partitions(const uint8_t buffer[], blk_partition_t* parts, size_t num_parts)
{
    size_t offs = ptbl_offs;
    for (size_t p = 0; p < num_parts; ++p)
    {
        blk_partition_t *part = &parts[p];
        part->bootable = buffer[mbr_ptbl_active + offs] & 0x80;
        part->boot_unit = buffer[mbr_ptbl_active + offs] & 0x7F;
        part->type = buffer[mbr_ptbl_type + offs];
        part->num_sectors = to_word(buffer, mbr_ptbl_sect_cnt + offs);
        part->start_sector = to_word(buffer, mbr_ptbl_lba + offs);
        offs += ptbl_size;
    }
}

//! Check if partition is extended
static bool is_extended(uint8_t type)
{
    return type == ext_linux_part || type == ext_part || type == ext_win98_part;
}

// MBR erase size
static int read_mbr_lfs_erase_size(uint8_t mbr_buf[], int part_no, size_t sect_size)
{
    if (part_no <= 0)
    {
        return -EINVAL;
    }
    if (sect_size <= MBR_ERASE_BLK_OFFSET + part_no)
    {
        return -ERANGE;
    }
    return mbr_buf[MBR_ERASE_BLK_OFFSET + part_no];
}


//! Parse extended partition
static int parse_extended(int disk, lba_t lba, blk_size_t count, struct partitions* outparts)
{    
    int error = 0;
    blk_dev_info_t diskinfo;
    error = blk_info(disk, &diskinfo);
    if(error<0) return error;
    int extended_part_num;
    blk_partition_t parts[num_parts];
    lba_t current_sector     = lba;
    unsigned long this_size = count * diskinfo.sector_size;

    uint32_t sector_in_buf = UINT32_MAX;
    uint8_t* sect_buf __attribute__((__cleanup__(free_clean_up))) = malloc(diskinfo.sector_size);
    if(!sect_buf) {
        printf("Unable to allocate memory in parse extended\n");
        return -ENOMEM;
    }

    size_t try_count = EXT_MAX_NUM_PARTS;
    while (try_count--) {
        if (sector_in_buf != current_sector) {
            printf("extended parse: Read sector %u\n", (unsigned)current_sector);
            error = blk_read(disk,current_sector,1, sect_buf);
            if (error < 0) break;
            sector_in_buf = current_sector;
        }
        {
            const uint8_t b1 = sect_buf[mbr_signature_offs];
            const uint8_t b2 = sect_buf[mbr_signature_offs + 1];
            if (b1 != 0x55 || b2 != 0xAA) {
                printf("extended_parse: No signature %02x,%02x\n", b1, b2);
                break;
            }
        }

        read_partitions(sect_buf, parts, num_parts);
        extended_part_num = -1;

        for (size_t partition_num = 0U; partition_num < num_parts; ++partition_num) {
            if (parts[partition_num].num_sectors == 0) {
                /* Partition is empty */
                continue;
            }
            if (is_extended(parts[partition_num].type)) {
                if (extended_part_num < 0)
                    extended_part_num = partition_num;
                continue; /* We'll examine this ext partition later */
            }

            /* Some sanity checks */
            const uint64_t poffset = parts[partition_num].start_sector * diskinfo.sector_size;
            const uint64_t psize   = parts[partition_num].num_sectors * diskinfo.sector_size;
            const uint64_t pnext   = current_sector * diskinfo.sector_size + poffset;
            if ((poffset + psize > this_size) ||                                  // oversized
                (pnext < lba * diskinfo.sector_size) ||        // going backward
                (pnext > (lba + count) * diskinfo.sector_size) // outsized
            ) {
                printf("parse_part_sanity: Part %d looks strange: current_sector %u offset %u next %u\n",
                            (int)partition_num,
                            (unsigned)current_sector,
                            (unsigned)poffset,
                            (unsigned)pnext);
                continue;
            }
            parts[partition_num].logical_num = partition_num + 1;
            parts[partition_num].start_sector = current_sector;
            memcpy(&outparts->part[outparts->nparts++], &parts[partition_num], sizeof(blk_partition_t));
            //m_parts.emplace_back(parts[partition_num]);
            try_count = EXT_MAX_NUM_PARTS;
        }

        if (extended_part_num < 0) {
            printf("parse_extended_parts: No more extended partitions\n");
            break; /* nothing left to do */
        }
        /* Examine the next extended partition */
        current_sector = lba + parts[extended_part_num].start_sector;
        this_size      = parts[extended_part_num].num_sectors * diskinfo.sector_size;
    }
    return error;
}

static bool check_partition(const blk_dev_info_t* disk_info, const blk_partition_t* part)
{
    const uint64_t this_size = (uint64_t)disk_info->sector_count * (uint64_t)disk_info->sector_size;
    const uint64_t poffset   = (uint64_t)part->start_sector * (uint64_t)disk_info->sector_size;
    const uint64_t psize     = (uint64_t)part->num_sectors * (uint64_t)disk_info->sector_size;
    const uint64_t pnext     = (uint64_t)part->start_sector * (uint64_t)disk_info->sector_size + poffset;
    if ((poffset + psize > this_size) ||                    // oversized
        (pnext < (uint64_t)part->start_sector * disk_info->sector_size) // going backward
    ) {
        printf("check_partitions: Part %d looks strange: start_sector %u offset %u next %u\n",
                    (unsigned)part->mbr_num,
                    (unsigned)part->start_sector,
                    (unsigned)poffset,
                    (unsigned)pnext);
        return false;
    }
    return true;
}


int blk_priv_scan_partitions(int disk, struct blk_partition** part)
{
    int ret;
    blk_dev_info_t diskinfo;
    ret = blk_info(disk, &diskinfo);
    if(ret<0) {
        printf("scan_part: Unable to read disk info error %i\n", ret);
        return ret;
    }
    uint8_t* mbr_sect __attribute__((__cleanup__(free_clean_up))) = malloc(diskinfo.sector_size);
    if(!mbr_sect) {
        printf("scan_part: Unable to allocate memory\n");
        return -ENOMEM;
    }

    ret = blk_read(disk,0,1,mbr_sect);
    if (ret < 0) {
        printf("scan_part: Unable to read disk block error %i\n", ret);
        return ret;
    }
    if (diskinfo.sector_size < min_sector_size)
    {
        printf("scan_partitions: Sector size %lu < min_sector size %u\n", 
            diskinfo.sector_size, min_sector_size);
        return -ENXIO;
    }
    // Check initial signature
    if ((mbr_sect[mbr_signature_offs] != 0x55) && (mbr_sect[mbr_signature_offs + 1] != 0xAA))
    {
        printf("scan_partitions: Invalid partition signature\n");
        return -ENXIO;
    }
    /* Copy the 4 partition records into partitions */
    blk_partition_t root_part[num_parts];
    read_partitions(mbr_sect, root_part, num_parts);
    // Add not extended partitions
    *part = reallocarray( *part, MAX_NUM_PARTS, sizeof(blk_partition_t) );
    struct partitions outparts = { *part, 1};
    for(size_t i=0; i<num_parts; ++i)
    {
        blk_partition_t* partx = &root_part[i];
        if (is_extended(partx->type))
            continue;

        if (!check_partition(&diskinfo, partx))
            continue;

        if (partx->num_sectors)
        {
            partx->logical_num = i + 1;
            partx->mbr_num = i + 1;
            memcpy(&outparts.part[outparts.nparts++], partx, sizeof(blk_partition_t));
            //m_parts.emplace_back(part);
        }
    }
    for (size_t i=0; i<num_parts; ++i)
    {
        blk_partition_t* partx = &root_part[i];
        if (is_extended(partx->type))
        {
            ret = parse_extended(disk,partx->start_sector, partx->num_sectors, &outparts);
            if (ret < 0)
                return ret;
        }
    }
    *part = reallocarray( *part, outparts.nparts, sizeof(blk_partition_t) );
    // Extra info about part size
    for(size_t p=1; p<outparts.nparts; ++p) {
        const int erase_siz = read_mbr_lfs_erase_size(mbr_sect,p,diskinfo.sector_size);
        (*part)[p].erase_blk = (erase_siz>0)?(1U << erase_siz):(0);
    }
    return outparts.nparts;
}