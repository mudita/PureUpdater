// For more information related to HAB library API,
// refer to HAB v4 API Reference Manual

#ifndef __HAB_H
#define __HAB_H

#include <stddef.h>
#include <stdint.h>

// HAB values

#define HAB_TAG_IVT 0xd1
#define HAB_TAG_DCD 0xd2
#define HAB_TAG_CSF 0xd4
#define HAB_TAG_CRT 0xd7
#define HAB_TAG_SIG 0xd8
#define HAB_TAG_EVT 0xdb
#define HAB_TAG_RVT 0xdd
#define HAB_TAG_WRP 0x81
#define HAB_TAG_MAC 0xac

#define HAB_STS_ANY 0x00
#define HAB_FAILURE 0x33
#define HAB_WARNING 0x69
#define HAB_SUCCESS 0xf0

#define HAB_RSN_ANY 0x00
#define HAB_ENG_FAIL 0x30
#define HAB_INV_ADDRESS 0x22
#define HAB_INV_ASSERTION 0x0c
#define HAB_INV_CALL 0x28
#define HAB_INV_CERTIFICATE 0x21
#define HAB_INV_COMMAND 0x06
#define HAB_INV_CSF 0x11
#define HAB_INV_DCD 0x27
#define HAB_INV_INDEX 0x0f
#define HAB_INV_IVT 0x05
#define HAB_INV_KEY 0x1d
#define HAB_INV_RETURN 0x1e
#define HAB_INV_SIGNATURE 0x18
#define HAB_INV_SIZE 0x17
#define HAB_MEM_FAIL 0x2e
#define HAB_OVR_COUNT 0x2b
#define HAB_OVR_STORAGE 0x2d
#define HAB_UNS_ALGORITHM 0x12
#define HAB_UNS_COMMAND 0x03
#define HAB_UNS_ENGINE 0x0a
#define HAB_UNS_ITEM 0x24
#define HAB_UNS_KEY 0x1b
#define HAB_UNS_PROTOCOL 0x14
#define HAB_UNS_STATE 0x09

#define HAB_CTX_ANY 0x00
#define HAB_CTX_ENTRY 0xe1
#define HAB_CTX_TARGET 0x33
#define HAB_CTX_AUTHENTICATE 0x0a
#define HAB_CTX_DCD 0xdd
#define HAB_CTX_CSF 0xcf
#define HAB_CTX_COMMAND 0xc0
#define HAB_CTX_AUT_DAT 0xdb
#define HAB_CTX_ASSERT 0xa0
#define HAB_CTX_EXIT 0xee

#define HAB_ENG_ANY 0x00
#define HAB_ENG_SCC 0x03
#define HAB_ENG_RTIC 0x05
#define HAB_ENG_SAHARA 0x06
#define HAB_ENG_CSU 0x0a
#define HAB_ENG_SRTC 0x0c
#define HAB_ENG_DCP 0x1b
#define HAB_ENG_CAAM 0x1d
#define HAB_ENG_SNVS 0x1e
#define HAB_ENG_OCOTP 0x21
#define HAB_ENG_DTCP 0x22
#define HAB_ENG_ROM 0x36
#define HAB_ENG_HDCP 0x24
#define HAB_ENG_SW 0xff

#define HAB_CFG_RETURN 0x33
#define HAB_CFG_OPEN 0xf0
#define HAB_CFG_CLOSED 0xcc

#define HAB_STATE_INITIAL 0x33
#define HAB_STATE_CHECK 0x55
#define HAB_STATE_NONSECURE 0x66
#define HAB_STATE_TRUSTED 0x99
#define HAB_STATE_SECURE 0xaa
#define HAB_STATE_FAIL_SOFT 0xcc
#define HAB_STATE_FAIL_HARD 0xff
#define HAB_STATE_NONE 0xf0

#define HAB_ASSERT_BLOCK 0x00

typedef uint8_t hab_tag_t;
typedef uint8_t hab_status_t;
typedef uint8_t hab_reason_t;
typedef uint8_t hab_ctx_t;
typedef uint8_t hab_engine_t;
typedef uint8_t hab_config_t;
typedef uint8_t hab_state_t;
typedef uint8_t hab_assertion_t;

typedef enum
{
    HAB_TGT_MEMORY = 0x0f,
    HAB_TGT_PERIPHERAL = 0xf0,
    HAB_TGT_ANY = 0x55
} hab_target_t;

// HAB function prototypes

typedef hab_status_t (*hab_entry_f)(void);
typedef hab_status_t (*hab_exit_f)(void);
typedef hab_status_t (*hab_check_target_f)(hab_target_t type, const void *start,
                                           size_t sz);
typedef void (*hab_image_entry_f)(void);
typedef hab_status_t (*hab_loader_callback_f)(void **inout_image_load_addr,
                                              size_t *inout_image_size, const void *boot_data);
typedef hab_image_entry_f (*hab_authenticate_image_f)(
    uint8_t cid,
    ptrdiff_t ivt_offset,
    void **inout_image_load_addr,
    size_t *inout_image_size,
    hab_loader_callback_f loader);
typedef hab_status_t (*hab_run_dcd_f)(const uint8_t *dcd);
typedef hab_status_t (*hab_run_csf_f)(const uint8_t *csf, uint8_t cid);
typedef hab_status_t (*hab_assert_f)(hab_assertion_t assert_type,
                                     const void *data, uint32_t data_size);
typedef hab_status_t (*hab_report_event_f)(hab_status_t status, uint32_t index,
                                           uint8_t *out_event, size_t *inout_event_size);
typedef hab_status_t (*hab_report_status_f)(hab_config_t *out_cfg,
                                            hab_state_t *out_state);
typedef void (*hab_failsafe_f)(void);
typedef hab_image_entry_f (*hab_authenticate_image_no_dcd_f)(
    uint8_t cid,
    ptrdiff_t ivt_offset,
    void **inout_image_load_addr,
    size_t *inout_image_size,
    hab_loader_callback_f loader);
typedef uint32_t (*hab_get_version_f)(void);
typedef hab_status_t (*hab_authenticate_container_f)(
    uint8_t cid,
    ptrdiff_t ivt_offset,
    void **inout_image_load_addr,
    size_t *inout_image_size,
    hab_loader_callback_f loader,
    uint32_t srk_mask,
    int skip_dcd);

// HAB structures

typedef struct __attribute__((packed))
{
    void *image_start;
    uint32_t image_size;
    uint32_t plugin_flag;
} hab_boot_data_t;

typedef struct __attribute__((packed))
{
    hab_tag_t tag;
    uint16_t len_be;
    uint8_t ver;
} hab_hdr_t;

typedef struct __attribute__((packed))
{
    hab_hdr_t hdr;
    uint8_t cmds[];
} hab_csf_t;

typedef struct __attribute__((packed))
{
    hab_hdr_t hdr;
    hab_image_entry_f entry;
    uint32_t reserved1;
    void *dcd;
    hab_boot_data_t *boot_data;
    void *self;
    hab_csf_t *csf;
    uint32_t reserved2;
} hab_ivt_t;

typedef struct __attribute((packed))
{
    hab_hdr_t hdr;
    hab_status_t status;
    hab_reason_t reason;
    hab_ctx_t ctx;
    uint8_t data[];
} hab_evt_t;

typedef struct __attribute__((packed))
{
    hab_hdr_t hdr;                                                 // offset: 0x00
    hab_entry_f hab_entry;                                         // offset: 0x04
    hab_exit_f hab_exit;                                           // offset: 0x08
    hab_check_target_f hab_check_target;                           // offset: 0x0c
    hab_authenticate_image_f hab_authenticate_image;               // offset: 0x10
    hab_run_dcd_f hab_run_dcd;                                     // offset: 0x14
    hab_run_csf_f hab_run_csf;                                     // offset: 0x18
    hab_assert_f hab_assert;                                       // offset: 0x1c
    hab_report_event_f hab_report_event;                           // offset: 0x20
    hab_report_status_f hab_report_status;                         // offset: 0x24
    hab_failsafe_f hab_failsafe;                                   // offset: 0x28
    hab_authenticate_image_no_dcd_f hab_authenticate_image_no_dcd; // offset: 0x2c
    hab_get_version_f hab_get_version;                             // offset: 0x30
    hab_authenticate_container_f hab_authenticate_container;       // offset: 0x34
} hab_rvt_t;

// HAB function wrappers

hab_status_t hab_entry(void);
hab_status_t hab_exit(void);
hab_status_t hab_check_target(hab_target_t type, const void *start, size_t sz);
hab_image_entry_f hab_authenticate_image(
    uint8_t cid,
    ptrdiff_t ivt_offset,
    void **inout_image_load_addr,
    size_t *inout_image_size,
    hab_loader_callback_f loader);
hab_status_t hab_run_dcd(const uint8_t *dcd);
hab_status_t hab_run_csf(const uint8_t *csf, uint8_t cid);
hab_status_t hab_assert(hab_assertion_t assert_type, const void *data,
                        uint32_t data_size);
hab_status_t hab_report_event(hab_status_t status, uint32_t index,
                              uint8_t *out_event, size_t *inout_event_size);
hab_status_t hab_report_status(hab_config_t *out_cfg,
                               hab_state_t *out_state);
void hab_failsafe(void);
hab_image_entry_f hab_authenticate_image_no_dcd(
    uint8_t cid,
    ptrdiff_t ivt_offset,
    void **inout_image_load_addr,
    size_t *inout_image_size,
    hab_loader_callback_f loader);
uint32_t hab_get_version(void);
hab_status_t hab_authenticate_container(
    uint8_t cid,
    ptrdiff_t ivt_offset,
    void **inout_image_load_addr,
    size_t *inout_image_size,
    hab_loader_callback_f loader,
    uint32_t srk_mask,
    int skip_dcd);

// Utility functions

uint16_t hab_to_le16(uint16_t v);
uint32_t hab_be_arr_to_u32(uint8_t arr[4]);
const char *hab_status_to_str(hab_status_t status);
const char *hab_reason_to_str(hab_reason_t reason);
const char *hab_ctx_to_str(hab_ctx_t ctx);
const char *hab_cfg_to_str(hab_config_t cfg);
const char *hab_state_to_str(hab_state_t state);

#endif
