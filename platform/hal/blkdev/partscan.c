#include <prv/blkdev/partscan.h>
#include <hal/blk_dev.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>


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

struct partitions {
    blk_partition_t *part;
    size_t nparts;
};

// ! For cleanup dynamic resources
static void cleanup_dynamic_alloc(void *mem)
{
    free(mem);
}

// TO word
static inline uint32_t to_word(const uint8_t vec[], size_t offs)
{
    const uint8_t* buf = &vec[offs];
    return ((uint32_t)buf[0] << 0U) | ((uint32_t)buf[1] << 8U) | 
    ((uint32_t)buf[2] << 16U) | ((uint32_t)buf[3] << 24U);
}

// TO short
inline uint16_t to_short(const uint8_t vec[], size_t offs)
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
    uint8_t* sect_buf __attribute__((__cleanup__(cleanup_dynamic_alloc))) = malloc(diskinfo.sector_size);

    size_t try_count = EXT_MAX_NUM_PARTS;
    while (try_count--) {
        if (sector_in_buf != current_sector) {
            //LOG_INFO("extended parse: Read sector %u\n", unsigned(current_sector));
            error = blk_read(disk,current_sector,1, sect_buf);
            if (error < 0) break;
            sector_in_buf = current_sector;
        }
        {
            const uint8_t b1 = sect_buf[mbr_signature_offs];
            const uint8_t b2 = sect_buf[mbr_signature_offs + 1];
            if (b1 != 0x55 || b2 != 0xAA) {
                //LOG_ERROR("extended_parse: No signature %02x,%02x", b1, b2);
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
                /*
                LOG_WARN("Part %d looks strange: current_sector %u offset %u next %u\n",
                            int(partition_num),
                            unsigned(current_sector),
                            unsigned(poffset),
                            unsigned(pnext));
                */
                continue;
            }
            parts[partition_num].logical_num = partition_num + 1;
            parts[partition_num].start_sector = current_sector;
            memcpy(&outparts->part[outparts->nparts++], &parts[partition_num], sizeof(blk_partition_t));
            //m_parts.emplace_back(parts[partition_num]);
            try_count = EXT_MAX_NUM_PARTS;
        }

        if (extended_part_num < 0) {
            //LOG_DEBUG("No more extended partitions");
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
        /*
        LOG_WARN("Part %d looks strange: start_sector %u offset %u next %u\n",
                    unsigned(part.mbr_number),
                    unsigned(part.start_sector),
                    unsigned(poffset),
                    unsigned(pnext));
        */
        return false;
    }
    return true;
}


int blk_priv_scan_partitions(int disk, struct blk_partition** part)
{
    int ret;
    blk_dev_info_t diskinfo;
    ret = blk_info(disk, &diskinfo);
    if(ret<0) return ret;
    uint8_t* mbr_sect __attribute__((__cleanup__(cleanup_dynamic_alloc))) = malloc(diskinfo.sector_size);
    if(!mbr_sect) return -ENOMEM;

    ret = blk_read(disk,0,1,mbr_sect);
    if (ret < 0) 
        return ret;
    if (diskinfo.sector_size < min_sector_size)
        return -ENXIO;
    // Check initial signature
    if ((mbr_sect[mbr_signature_offs] != 0x55) && (mbr_sect[mbr_signature_offs + 1] != 0xAA))
        return -ENXIO;
    /* Copy the 4 partition records into partitions */
    blk_partition_t root_part[num_parts];
    read_partitions(mbr_sect, root_part, num_parts);
    // Add not extended partitions
    struct partitions outparts = { *part, 1};
    for(size_t i=0; i<num_parts; ++i)
    {
        blk_partition_t* part = &root_part[i];
        if (is_extended(part->type))
            continue;

        if (!check_partition(&diskinfo, part))
            continue;

        if (part->num_sectors)
        {
            part->logical_num = i + 1;
            part->mbr_num = i + 1;
            memcpy(&outparts.part[outparts.nparts++], part, sizeof(blk_partition_t));
            //m_parts.emplace_back(part);
        }
    }
    for (size_t i=0; i<num_parts; ++i)
    {
        blk_partition_t* part = &root_part[i];
        if (is_extended(part->type))
        {
            ret = parse_extended(disk,part->start_sector, part->num_sectors, &outparts);
            if (ret < 0)
                return ret;
        }
    }
    return outparts.nparts;
}