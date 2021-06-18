#pragma once


// Forward decl
struct _mmc_card;

/** Initialize emmc card subsystem
 * @return error code
 */
int emmc_init();

/** Get emmc card from master driver
 * @return emmc card structure or null
 */
struct _mmc_card* emmc_card();