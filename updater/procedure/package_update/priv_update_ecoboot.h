#pragma once

/** Update the ecoboot from the seleted file
 * @param[in] path Path to ecoboot binary file
 * @return 0 if success -errno on failure
 */
int ecoboot_update(const char *path);
