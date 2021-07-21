#include <drivers/hab/hab.h>

#ifdef CPU_MIMXRT1051DVL6B
static const hab_rvt_t *const m_hab_rvt = (hab_rvt_t *)0x00200300;
#else
#error "Unknown HAB RVT address (unknown or unspecified CPU)"
#endif

hab_status_t hab_entry(void)
{
    return m_hab_rvt->hab_entry();
}

hab_status_t hab_exit(void)
{
    return m_hab_rvt->hab_exit();
}

hab_status_t hab_check_target(hab_target_t type, const void *start, size_t sz)
{
    return m_hab_rvt->hab_check_target(type, start, sz);
}

hab_image_entry_f hab_authenticate_image(
    uint8_t cid,
    ptrdiff_t ivt_offset,
    void **inout_image_load_addr,
    size_t *inout_image_size,
    hab_loader_callback_f loader)
{
    return m_hab_rvt->hab_authenticate_image(cid, ivt_offset, inout_image_load_addr,
                                             inout_image_size, loader);
}

hab_status_t hab_run_dcd(const uint8_t *dcd)
{
    return m_hab_rvt->hab_run_dcd(dcd);
}

hab_status_t hab_run_csf(const uint8_t *csf, uint8_t cid)
{
    return m_hab_rvt->hab_run_csf(csf, cid);
}

hab_status_t hab_assert(hab_assertion_t assert_type, const void *data,
                        uint32_t data_size)
{
    return m_hab_rvt->hab_assert(assert_type, data, data_size);
}

hab_status_t hab_report_event(hab_status_t status, uint32_t index,
                              uint8_t *out_event, size_t *inout_event_size)
{
    return m_hab_rvt->hab_report_event(status, index, out_event, inout_event_size);
}

hab_status_t hab_report_status(hab_config_t *out_cfg,
                               hab_state_t *out_state)
{
    return m_hab_rvt->hab_report_status(out_cfg, out_state);
}

void hab_failsafe(void)
{
    m_hab_rvt->hab_failsafe();
}

hab_image_entry_f hab_authenticate_image_no_dcd(
    uint8_t cid,
    ptrdiff_t ivt_offset,
    void **inout_image_load_addr,
    size_t *inout_image_size,
    hab_loader_callback_f loader)
{
    return m_hab_rvt->hab_authenticate_image_no_dcd(cid, ivt_offset,
                                                    inout_image_load_addr, inout_image_size, loader);
}

uint32_t hab_get_version(void)
{
    return m_hab_rvt->hab_get_version();
}

hab_status_t hab_authenticate_container(
    uint8_t cid,
    ptrdiff_t ivt_offset,
    void **inout_image_load_addr,
    size_t *inout_image_size,
    hab_loader_callback_f loader,
    uint32_t srk_mask,
    int skip_dcd)
{
    return m_hab_rvt->hab_authenticate_container(cid, ivt_offset, inout_image_load_addr,
                                                 inout_image_size, loader, srk_mask, skip_dcd);
}

uint16_t hab_to_le16(uint16_t v)
{
    return v >> 8 | v << 8;
}

uint32_t hab_be_arr_to_u32(uint8_t arr[4])
{
    return arr[0] << 24 | arr[1] << 16 | arr[2] << 8 | arr[3];
}

const char *hab_status_to_str(hab_status_t status)
{
    switch (status)
    {
    case HAB_STS_ANY:
        return "ANY";
    case HAB_FAILURE:
        return "FAILURE";
    case HAB_WARNING:
        return "WARNING";
    case HAB_SUCCESS:
        return "SUCCESS";
    default:
        return "[unknown]";
    };
}

const char *hab_reason_to_str(hab_reason_t reason)
{
    switch (reason)
    {
    case HAB_RSN_ANY:
        return "ANY";
    case HAB_ENG_FAIL:
        return "ENG_FAIL";
    case HAB_INV_ADDRESS:
        return "INV_ADDRESS";
    case HAB_INV_ASSERTION:
        return "INV_ASSERTION";
    case HAB_INV_CALL:
        return "INV_CALL";
    case HAB_INV_CERTIFICATE:
        return "INV_CERTIFICATE";
    case HAB_INV_COMMAND:
        return "INV_COMMAND";
    case HAB_INV_CSF:
        return "INV_CSF";
    case HAB_INV_DCD:
        return "INV_DCD";
    case HAB_INV_INDEX:
        return "INV_INDEX";
    case HAB_INV_IVT:
        return "INV_IVT";
    case HAB_INV_KEY:
        return "INV_KEY";
    case HAB_INV_RETURN:
        return "INV_RETURN";
    case HAB_INV_SIGNATURE:
        return "INV_SIGNATURE";
    case HAB_INV_SIZE:
        return "INV_SIZE";
    case HAB_MEM_FAIL:
        return "MEM_FAIL";
    case HAB_OVR_COUNT:
        return "OVR_COUNT";
    case HAB_OVR_STORAGE:
        return "OVR_STORAGE";
    case HAB_UNS_ALGORITHM:
        return "UNS_ALGORITHM";
    case HAB_UNS_COMMAND:
        return "UNS_COMMAND";
    case HAB_UNS_ENGINE:
        return "UNS_ENGINE";
    case HAB_UNS_ITEM:
        return "UNS_ITEM";
    case HAB_UNS_KEY:
        return "UNS_KEY";
    case HAB_UNS_PROTOCOL:
        return "UNS_PROTOCOL";
    case HAB_UNS_STATE:
        return "UNS_STATE";
    default:
        return "[unknown]";
    }
}

const char *hab_ctx_to_str(hab_ctx_t ctx)
{
    switch (ctx)
    {
    case HAB_CTX_ANY:
        return "ANY";
    case HAB_CTX_ENTRY:
        return "ENTRY";
    case HAB_CTX_TARGET:
        return "TARGET";
    case HAB_CTX_AUTHENTICATE:
        return "AUTHENTICATE";
    case HAB_CTX_DCD:
        return "DCD";
    case HAB_CTX_CSF:
        return "CSF";
    case HAB_CTX_COMMAND:
        return "COMMAND";
    case HAB_CTX_AUT_DAT:
        return "AUT_DAT";
    case HAB_CTX_ASSERT:
        return "ASSERT";
    case HAB_CTX_EXIT:
        return "EXIT";
    default:
        return "[unknown]";
    }
}

const char *hab_cfg_to_str(hab_config_t cfg)
{
    switch (cfg)
    {
    case HAB_CFG_RETURN:
        return "FIELD_RETURN";
    case HAB_CFG_OPEN:
        return "OPEN";
    case HAB_CFG_CLOSED:
        return "CLOSED";
    default:
        return "[unknown]";
    }
}

const char *hab_state_to_str(hab_state_t state)
{
    switch (state)
    {
    case HAB_STATE_INITIAL:
        return "INITIAL";
    case HAB_STATE_CHECK:
        return "CHECK";
    case HAB_STATE_NONSECURE:
        return "NONSECURE";
    case HAB_STATE_TRUSTED:
        return "TRUSTED";
    case HAB_STATE_SECURE:
        return "SECURE";
    case HAB_STATE_FAIL_SOFT:
        return "FAIL_SOFT";
    case HAB_STATE_FAIL_HARD:
        return "FAIL_HARD";
    case HAB_STATE_NONE:
        return "NONE";
    default:
        return "[unknown]";
    }
}
