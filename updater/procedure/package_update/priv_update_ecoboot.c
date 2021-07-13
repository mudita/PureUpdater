#include "priv_update_ecoboot.h"
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <hal/blk_dev.h>

//! Ecooboot partition handle
#define ECO_PARTITION blk_disk_handle(blkdev_emmc_boot1, 0)

//! For cleanup dynamic resources
static void free_clean_up(uint8_t **ptr)
{
    free(*ptr);
}

//! Cleanup file descriptor
static void file_clean_up(FILE **fil)
{
    if (*fil)
    {
        fclose(*fil);
    }
}

/** Open ecoboot file in read only mode
 * and verify ecoboot header
 * @param[in] path Path to the ecoboot image
 * @param[out] file STDIO file handle
 * @return if positive image size otherwise error
 */
static int open_ecoboot_file(const char *path, FILE **file)
{
    const off_t minimal_size = 64 * 1024;
    struct stat st;
    if (!file)
    {
        return -EINVAL;
    }
    if (stat(path, &st))
    {
        printf("%s: Unable to stat errno %i\n", __PRETTY_FUNCTION__, errno);
        return -errno;
    }
    if (!S_ISREG(st.st_mode))
    {
        printf("%s: Is not regular file\n", __PRETTY_FUNCTION__);
        return -ENOEXEC;
    }
    if (st.st_size < minimal_size)
    {
        printf("%s: Bootloader size is too small\n", __PRETTY_FUNCTION__);
        return -ENOEXEC;
    }
    *file = fopen(path, "r");
    if (!*file)
    {
        printf("%s: Unable to open file %i\n", __PRETTY_FUNCTION__, errno);
        return -errno;
    }
    return st.st_size;
}

/** Flash the ecoboot device
 * @param[in] path Ecoboot path
 * @return error code
 */
static int flash_ecoboot(const char *path)
{
    blk_dev_info_t blk;
    int err, fil_size;
    FILE *fil __attribute__((__cleanup__(file_clean_up))) = NULL;
    if ((err = blk_info(ECO_PARTITION, &blk)))
    {
        printf("%s: Unable to get part block %i\n", __PRETTY_FUNCTION__, err);
        return err;
    }
    uint8_t *sect_buf __attribute__((__cleanup__(free_clean_up))) = malloc(blk.sector_size);
    if (!sect_buf)
    {
        printf("%s: No memory\n", __PRETTY_FUNCTION__);
        return -ENOMEM;
    }

    if ((fil_size = open_ecoboot_file(path, &fil)) <= 0)
    {
        return fil_size;
    }
    for (lba_t lba = 0, sz = fil_size; !feof(fil) && !ferror(fil);)
    {
        if (sz < blk.sector_size)
        {
            memset(sect_buf, 0, blk.sector_size);
            if (!fread(sect_buf, sz, 1, fil))
            {
                break;
            }
            sz = 0;
        }
        else
        {
            if (!fread(sect_buf, blk.sector_size, 1, fil))
            {
                if (!ferror(fil))
                {
                    sz -= blk.sector_size;
                    continue;
                }
                else
                {
                    break;
                }
            }
            sz -= blk.sector_size;
        }
        if ((err = blk_write(ECO_PARTITION, lba++, 1, sect_buf)))
        {
            return err;
        }
    }
    if (ferror(fil))
    {
        return -errno;
    }
    else
    {
        return 0;
    }
}

/** Flash the ecoboot device
 * @param[in] path Ecoboot path
 * @return error code
 */
static int verify_ecoboot(const char *path)
{
    blk_dev_info_t blk;
    int err, fil_size;
    FILE *fil __attribute__((__cleanup__(file_clean_up))) = NULL;
    if ((err = blk_info(ECO_PARTITION, &blk)))
    {
        printf("%s: Unable to get part block %i\n", __PRETTY_FUNCTION__, err);
        return err;
    }
    uint8_t *buf1 __attribute__((__cleanup__(free_clean_up))) = malloc(blk.sector_size);
    if (!buf1)
    {
        printf("%s: No memory\n", __PRETTY_FUNCTION__);
        return -ENOMEM;
    }
    uint8_t *buf2 __attribute__((__cleanup__(free_clean_up))) = malloc(blk.sector_size);
    if (!buf2)
    {
        printf("%s: No memory\n", __PRETTY_FUNCTION__);
        return -ENOMEM;
    }
    if ((fil_size = open_ecoboot_file(path, &fil)) <= 0)
    {
        return fil_size;
    }
    for (lba_t lba = 0, sz = fil_size; !feof(fil) && !ferror(fil);)
    {
        if ((err = blk_read(ECO_PARTITION, lba++, 1, buf1)))
        {
            return err;
        }

        if (sz < blk.sector_size)
        {
            memset(buf2, 0, blk.sector_size);
            if (!fread(buf2, sz, 1, fil))
            {
                break;
            }
            sz = 0;
        }
        else
        {
            if (!fread(buf2, blk.sector_size, 1, fil))
            {
                if (!ferror(fil))
                {
                    sz -= blk.sector_size;
                    continue;
                }
                else
                {
                    break;
                }
            }
            sz -= blk.sector_size;
        }
        if (memcmp(buf1, buf2, blk.sector_size) != 0)
        {
            printf("%s: Ecoboot verication failed\n", __PRETTY_FUNCTION__);
            return -EIO;
        }
    }
    if (ferror(fil))
    {
        return -errno;
    }
    else
    {
        return 0;
    }
}

// Update the ecoboot
int ecoboot_update(const char *path)
{
    int ret;
    do
    {
        ret = flash_ecoboot(path);
        if (ret)
        {
            break;
        }
        ret = verify_ecoboot(path);
        if (ret)
        {
            break;
        }
    } while (0);
    return ret;
}
