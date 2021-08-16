#pragma once

#include <common/trace.h>

//! Error codes
enum ecoboot_update_errors
{
    error_eco_update_ok,
    // With errno
    error_eco_vfs,    //! Errno related to vfs
    error_eco_blkdev, //! Errno related to blkdev
    error_eco_system, //! Generic system error
    //Without errno
    error_eco_update_no_reg_file, //! Not a regular file
    error_eco_too_small,          //! Bootloader too small
    error_eco_verify              //! Verification failed
};

/** Update the ecoboot from the seleted file
 * @param[in] workdir Working directory
 * @param[in] filename Filename with ecoboot.bin file
 * @param[in] tl Logger object
 * @return 0 if success -errno on failure
 */
int ecoboot_update(const char *workdir, const char *filename, trace_list_t *tl);

/** Check if there is ecoboot in the package
 * @param[in] workdir Working directory
 * @param[in] filename Filename with ecoboot.bin file
 * @param[in] tl Logger object
 * @return 1 if ecoboot in package, 0 if not -errno on failure
 */
int ecoboot_in_package(const char *workdir, const char *filename);
