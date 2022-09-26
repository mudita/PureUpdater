#include <hal/boot_reason.h>

#include <fsl_snvs_lp.h>
#include <stdio.h>

/** Get system boot reason code */
enum system_boot_reason_code system_boot_reason(void)
{
    static uint32_t boot_code;
    if (SNVS->LPGPR[0] != 0)
    {
        boot_code = SNVS->LPGPR[0];
        SNVS->LPGPR[0] = 0;
    }
    switch (boot_code)
    {
    case eco_update_code:
        return system_boot_reason_update;
    case eco_recovery_code:
        return system_boot_reason_recovery;
    case eco_factory_rst_code:
        return system_boot_reason_factory;
    case eco_factory_pgm_keys_code:
        return system_boot_reason_pgm_keys;
    default:
        return system_boot_reason_unknown;
    }
}

// Get the system boot reason str
const char *system_boot_reason_str(enum system_boot_reason_code code)
{
    switch (code)
    {
    case system_boot_reason_update:
        return "system_boot_reason_update";
    case system_boot_reason_recovery:
        return "system_boot_reason_recovery";
    case system_boot_reason_factory:
        return "system_boot_reason_factory";
    case system_boot_reason_unknown:
        return "system_boot_reason_unknown";
    case system_boot_reason_pgm_keys:
        return "system_boot_reason_pgm_keys";
    default:
        return "not in enum system_boot_reason_code";
    }
}
