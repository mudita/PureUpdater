#pragma once
#include <stdint.h>

/** Initialize the security engines
 * @return error codes
 */
int sec_initialize(void);

/** Check if the secure configuration is open
 * @return 1 - open 0 - closed , negative error
 */
int sec_configuration_is_open(void);

//! SRK key structure
struct sec_srk_key
{
    uint32_t srk[8];
};

/** Burn SRK key from file
 * @param[in] srk_key SRK key structure
 * @return 0 on success error whene negative
 * @note THIS IS PERMANENT & IRREVERSIBLE!
 */
int sec_burn_srk_keys(const struct sec_srk_key *srk_key);

/** Validate SRK key 
 * @param[in] srk_key SRK key for validate
 * @return 0 on sucess otherwise error
 * @note THIS IS PERMANENT & IRREVERSIBLE!
 */
int sec_verify_efuses(const struct sec_srk_key *srk_key);

/** 
 * Security lock bootloader prevent loading usigned binaries
 */
void sec_lock_bootloader(void);
