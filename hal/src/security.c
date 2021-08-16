/** HAB updater security module
 * Author: Lucjan Bryndza
 */
#include <hal/security.h>
#include <drivers/hab/hab.h>
#include <drivers/fsl_dcp.h>
#include <drivers/fsl_snvs_hp.h>
#include <drivers/fsl_snvs_lp.h>
#include <MIMXRT1051.h>
#include <errno.h>
#include <stdio.h>

#define SEC_CONFIG1_MASK 0x02

// ! Burn single efuse bit
static void burn_efuse_word(uint32_t ocotp_ctrl_addr, uint32_t data)
{
    // IPG_CLK=150MHz, refer to "Chapter 42.3.3 OTP Read/Write Timing Parameters"
    const uint32_t timing = OCOTP_TIMING_STROBE_PROG(1507) | OCOTP_TIMING_RELAX(3) | OCOTP_TIMING_STROBE_READ(14) | OCOTP_TIMING_WAIT(27);
    OCOTP->TIMING = timing;

    // wait for busy and error flags to clear
    while ((OCOTP->CTRL & (1 << OCOTP_CTRL_BUSY_SHIFT)) || (OCOTP->CTRL & (1 << OCOTP_CTRL_ERROR_SHIFT)))
    {
    }

    uint32_t ocotp_ctrl = OCOTP->CTRL;
    ocotp_ctrl &= ~OCOTP_CTRL_ADDR_MASK;
    ocotp_ctrl |= ocotp_ctrl_addr & OCOTP_CTRL_ADDR_MASK;
    ocotp_ctrl &= ~OCOTP_CTRL_WR_UNLOCK_MASK;
    ocotp_ctrl |= OCOTP_CTRL_WR_UNLOCK(0x3E77);

    OCOTP->CTRL = ocotp_ctrl; // write ADDR and unlock
    OCOTP->DATA = data;       // burn data into eFuses
}

// Check if configuration is open
int sec_configuration_is_open(void)
{
    return (OCOTP->CFG5 & SEC_CONFIG1_MASK) != SEC_CONFIG1_MASK;
}

// Burn efuses
int sec_burn_srk_keys(const struct sec_srk_key *srk_key)
{
    if (!srk_key)
    {
        return -EINVAL;
    }
    // Burn efuses
    for (size_t n = 0, addr = 0x18; n < ARRAY_SIZE(srk_key->srk); ++n, ++addr)
    {
        burn_efuse_word(OCOTP_CTRL_ADDR(addr), srk_key->srk[n]);
    }
    return 0;
}

// Lock bootloader
void sec_lock_bootloader(void)
{
    burn_efuse_word(OCOTP_CTRL_ADDR(0x06), SEC_CONFIG1_MASK);
}

// Validate efuses if they are correct
int sec_verify_efuses(const struct sec_srk_key *srk_key)
{
    for (size_t n = 0; n < ARRAY_SIZE(srk_key->srk); ++n)
    {
        uint32_t val;
        switch (n)
        {
        case 0:
            val = OCOTP->SRK0;
            break;
        case 1:
            val = OCOTP->SRK1;
            break;
        case 2:
            val = OCOTP->SRK2;
            break;
        case 3:
            val = OCOTP->SRK3;
            break;
        case 4:
            val = OCOTP->SRK4;
            break;
        case 5:
            val = OCOTP->SRK5;
            break;
        case 6:
            val = OCOTP->SRK6;
            break;
        case 7:
            val = OCOTP->SRK7;
            break;
        default:
            return -EINVAL;
        }
        if (srk_key->srk[n] != val)
        {
            return -EIO;
        }
    }
    return 0;
}

// Global state if security module is intialized
static bool g_hab_initialized;

// Initialize the security engine
int sec_initialize(void)
{
    if (sec_configuration_is_open())
    {
        printf("Open configuration skip HAB initialization skipped...\n");
    }
    else
    {
        hab_status_t hab_status;
        if ((hab_status = hab_entry()) != HAB_SUCCESS)
        {
            printf("Unable to initialize HAB sec engine status: %s\n", hab_status_to_str(hab_status));
            return -EIO;
        }
        if (g_hab_initialized)
        {
            printf("HAB already initialized\n");
            return -EEXIST;
        }
        g_hab_initialized = true;
        const uint32_t hab_version = hab_get_version();
        hab_config_t hab_config;
        hab_state_t hab_state;
        if ((hab_status = hab_report_status(&hab_config, &hab_state)) == HAB_SUCCESS || hab_status == HAB_WARNING)
        {
            printf("HAB status: version = 0x%08lx, config = %s, state = %s\n",
                   hab_version, hab_cfg_to_str(hab_config), hab_state_to_str(hab_state));
        }
        else
        {
            printf("%s: Error unable to get HAB version = 0x%08lx status = %s\n",
                   __PRETTY_FUNCTION__, hab_version, hab_status_to_str(hab_status));
            return -EIO;
        }
    }
    {
        // DCP init
        dcp_config_t dcp_config;
        DCP_GetDefaultConfig(&dcp_config);
        DCP_Init(DCP, &dcp_config);
    }
    return 0;
}

/** Security destructor
 * we need to ensure that the HAB is deintialized when updater
 * terminates
 */
void sec_deinitialize(void)
{
    if (g_hab_initialized)
    {
        hab_exit();
        g_hab_initialized = false;
    }
}
