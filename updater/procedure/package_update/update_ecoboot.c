#include "update_ecoboot.h"
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <hal/blk_dev.h>

//! Ecooboot partition handle
#define ECO_PARTITION blk_disk_handle(blkdev_emmc_boot1, 0)
// Number of sectors per transfer
#define SECTORS_PER_TRANSFER 32

//! For cleanup dynamic resources
static void free_clean_up(uint8_t **ptr)
{
    free(*ptr);
}

// Char cleanup
static void free_str_clean_up(char **ptr)
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
//! Error related translator
static const char *strerror_pgm_keys(int err)
{
    switch ((enum ecoboot_update_errors)err)
    {
    case error_eco_update_ok:
        return "Ecoboot update OK";
    case error_eco_vfs:
        return "VFS error";
    case error_eco_blkdev:
        return "Block device error";
    case error_eco_system:
        return "System error";
    case error_eco_update_no_reg_file:
        return "It is not a regular file";
    case error_eco_too_small:
        return "Ecoboot file to small";
    case error_eco_verify:
        return "Ecoboot verifcation error";
    default:
        return "";
    }
}

//! Extended error
static const char *strerror_ext_pgm_keys(int err, int err_ext)
{
    return (err >= error_eco_vfs && err <= error_eco_system) ? (strerror(-err_ext)) : ("");
}

/** Open ecoboot file in read only mode
 * and verify ecoboot header
 * @param[in] path Path to the ecoboot image
 * @param[out] file STDIO file handle
 * @param[in] trace Log trace object
 * @return if positive image size otherwise error
 */
static int open_ecoboot_file(const char *path, FILE **file, trace_t *trace)
{
    const off_t minimal_size = 64 * 1024;
    struct stat st;
    if (!file)
    {
        printf("%s: Filename not provided\n", __PRETTY_FUNCTION__);
        return -EINVAL;
    }
    if (stat(path, &st))
    {
        trace_write(trace, error_eco_vfs, -errno);
        printf("%s: File %s doesnt exists\n", __PRETTY_FUNCTION__, path);
        return -errno;
    }
    if (!S_ISREG(st.st_mode))
    {
        trace_write(trace, error_eco_update_no_reg_file, 0);
        return -ENOEXEC;
    }
    if (st.st_size < minimal_size)
    {
        trace_write(trace, error_eco_too_small, 0);
        return -ENOEXEC;
    }
    *file = fopen(path, "r");
    if (!*file)
    {
        trace_write(trace, error_eco_vfs, -errno);
        return -errno;
    }
    return st.st_size;
}

/** Flash the ecoboot device
 * @param[in] path Ecoboot path
 * @param[in] trace Log trace object
 * @return error code
 */
static int flash_ecoboot(const char *path, trace_t *trace)
{
    blk_dev_info_t blk;
    int err, fil_size;
    FILE *fil __attribute__((__cleanup__(file_clean_up))) = NULL;
    if ((err = blk_info(ECO_PARTITION, &blk)))
    {
        trace_write(trace, error_eco_blkdev, err);
        return err;
    }
    uint8_t *sect_buf __attribute__((__cleanup__(free_clean_up))) = malloc(blk.sector_size * SECTORS_PER_TRANSFER);
    if (!sect_buf)
    {
        trace_write(trace, error_eco_system, -errno);
        return -errno;
    }

    if ((fil_size = open_ecoboot_file(path, &fil, trace)) <= 0)
    {
        return fil_size;
    }
    for (lba_t lba = 0, sz = fil_size; !feof(fil) && !ferror(fil);)
    {
        if (sz < blk.sector_size * SECTORS_PER_TRANSFER)
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
            const size_t nread = fread(sect_buf, blk.sector_size, SECTORS_PER_TRANSFER, fil);
            if (nread == 0)
            {
                if (!ferror(fil))
                {
                    sz -= blk.sector_size * nread;
                    continue;
                }
                else
                {
                    break;
                }
            }
            sz -= blk.sector_size * nread;
        }
        if ((err = blk_write(ECO_PARTITION, lba, SECTORS_PER_TRANSFER, sect_buf)))
        {
            trace_write(trace, error_eco_blkdev, err);
            return err;
        }
        lba += SECTORS_PER_TRANSFER;
    }
    if (ferror(fil))
    {
        trace_write(trace, error_eco_vfs, -errno);
        return -errno;
    }
    else
    {
        return 0;
    }
}

/** Flash the ecoboot device
 * @param[in] path Ecoboot path
 * @param[in] trace Log trace object
 * @return error code
 */
static int verify_ecoboot(const char *path, trace_t *trace)
{
    blk_dev_info_t blk;
    int err, fil_size;
    FILE *fil __attribute__((__cleanup__(file_clean_up))) = NULL;
    if ((err = blk_info(ECO_PARTITION, &blk)))
    {
        trace_write(trace, error_eco_blkdev, err);
        return err;
    }
    uint8_t *buf1 __attribute__((__cleanup__(free_clean_up))) = malloc(blk.sector_size * SECTORS_PER_TRANSFER);
    if (!buf1)
    {
        trace_write(trace, error_eco_system, -errno);
        return -errno;
    }
    uint8_t *buf2 __attribute__((__cleanup__(free_clean_up))) = malloc(blk.sector_size * SECTORS_PER_TRANSFER);
    if (!buf2)
    {
        trace_write(trace, error_eco_system, -errno);
        return -errno;
    }
    if ((fil_size = open_ecoboot_file(path, &fil, trace)) <= 0)
    {
        return fil_size;
    }
    for (lba_t lba = 0, sz = fil_size; !feof(fil) && !ferror(fil);)
    {
        if ((err = blk_read(ECO_PARTITION, lba, SECTORS_PER_TRANSFER, buf1)))
        {
            trace_write(trace, error_eco_blkdev, err);
            return err;
        }
        lba += SECTORS_PER_TRANSFER;

        if (sz < blk.sector_size * SECTORS_PER_TRANSFER)
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
            const size_t nread = fread(buf2, blk.sector_size, SECTORS_PER_TRANSFER, fil);
            if (nread == 0)
            {
                if (!ferror(fil))
                {
                    sz -= blk.sector_size * nread;
                    continue;
                }
                else
                {
                    break;
                }
            }
            sz -= blk.sector_size * nread;
        }
        if (memcmp(buf1, buf2, blk.sector_size * SECTORS_PER_TRANSFER) != 0)
        {
            trace_write(trace, error_eco_verify, 0);
            return error_eco_verify;
        }
    }
    if (ferror(fil))
    {
        trace_write(trace, error_eco_vfs, -errno);
        return -errno;
    }
    else
    {
        return 0;
    }
}

// Update the ecoboot
int ecoboot_update(const char *workdir, const char *filename, trace_list_t *tl)
{
    int ret;
    char *path __attribute__((__cleanup__(free_str_clean_up))) = malloc(strlen(workdir) + strlen(filename) + sizeof("/"));
    trace_t *trace = trace_append(__PRETTY_FUNCTION__, tl, strerror_pgm_keys, strerror_ext_pgm_keys);
    // Append path
    strcpy(path, workdir);
    strcat(path, "/");
    strcat(path, filename);
    do
    {
        // Program flash
        ret = flash_ecoboot(path, trace);
        if (ret)
        {
            break;
        }
        // Verify flaash
        ret = verify_ecoboot(path, trace);
        if (ret)
        {
            break;
        }
        // Remove file
        ret = unlink(path);
        if (ret)
        {
            ret = error_eco_vfs;
            break;
        }
    } while (0);
    return ret;
}

int ecoboot_in_package(const char *workdir, const char *filename)
{
    char *path __attribute__((__cleanup__(free_str_clean_up))) = malloc(strlen(workdir) + strlen(filename) + sizeof("/") + 1);

    struct stat st;
    if (!filename) {
        printf("%s: Filename not provided\n", __PRETTY_FUNCTION__);
        return -EINVAL;
    }

    sprintf(path, "%s/%s", workdir, filename);

    if (stat(path, &st)) {
        printf("%s: File %s %s\n", __PRETTY_FUNCTION__, path, strerror(errno));
        if (errno == EEXIST) {
            return 0;
        }
        return -errno;
    }
    return 1;
}
