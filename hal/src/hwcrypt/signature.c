#include <hal/hwcrypt/signature.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <drivers/hab/hab.h>
#include <hal/security.h>
#include <stdint.h>

//! Maximum allowed file size
#define IVT_OFFSET 0x400

static __attribute__((section(".signaturespace"))) uint8_t signature[65535];
// Cleanup file descriptor
static void file_clean_up(FILE **fil)
{
    if (*fil)
    {
        fclose(*fil);
    }
}

// Cleanup allocated memory
static void free_clean_up(uint8_t **ptr)
{
    free(*ptr);
}

// Load file into allocated buffer
static void *load_file_int_sig_space(const char *path)
{
    FILE *filp __attribute__((__cleanup__(file_clean_up))) = fopen(path, "rb");
    if (!filp)
    {
        return NULL;
    }
    if (fseek(filp, 0, SEEK_END))
    {
        return NULL;
    }
    const long fil_siz = ftell(filp);
    if (fseek(filp, 0, SEEK_SET))
    {
        return NULL;
    }
    if ((size_t)fil_siz > sizeof signature)
    {
        errno = -E2BIG;
        return NULL;
    }
    if (fread(signature, fil_siz, 1, filp) != 1)
    {
        return NULL;
    }
    printf("After fread\n");
    return signature;
}

//! Debug only
static void Print_Image_Info(const hab_ivt_t *ivt)
{
    printf("IVT ver. 0x%02x:\n", ivt->hdr.ver);
    printf("    entry:     0x%08x\n", (unsigned)ivt->entry);
    printf("    dcd:       0x%08x\n", (unsigned)ivt->dcd);
    printf("    boot_data: 0x%08x\n", (unsigned)ivt->boot_data);
    printf("    csf:       0x%08x\n", (unsigned)ivt->csf);

    if (ivt->boot_data != NULL)
    {
        printf("Boot data:\n");
        printf("image_start: 0x%08x\n", (unsigned)ivt->boot_data->image_start);
        printf("image_size:  0x%x\n", (unsigned)ivt->boot_data->image_size);
        printf("plugin_flag: %u\n", (unsigned)ivt->boot_data->plugin_flag);
    }

    if (ivt->csf)
    {
        printf("CSF ver. 0x%02x\n", ivt->csf->hdr.ver);
    }
}

void HAB_Print_Audit_Log(void)
{
    uint32_t hab_event_index = 0;
    for (hab_status_t hab_status = HAB_SUCCESS; hab_status == HAB_SUCCESS; ++hab_event_index)
    {
        uint8_t hab_evt_buf[32];
        const hab_evt_t *const hab_evt = (hab_evt_t *)hab_evt_buf;
        size_t hab_evt_size = sizeof(hab_evt_buf);
        hab_status = hab_report_event(HAB_STS_ANY, hab_event_index, hab_evt_buf,
                                      &hab_evt_size);

        if (hab_status == HAB_SUCCESS)
        {
            if (hab_event_index == 0)
            {
                printf("HAB audit log:\n");
            }
            printf("    event %lu: status = %s, reason = %s, context = %s\n",
                   hab_event_index, hab_status_to_str(hab_evt->status),
                   hab_reason_to_str(hab_evt->reason), hab_ctx_to_str(hab_evt->ctx));
        }
    }
}

static hab_status_t authenticate_image(const hab_ivt_t *ivt)
{
    // Authenticate application image
    const ptrdiff_t ivt_offset = ivt->self - ivt->boot_data->image_start;
    void *image_load_addr = ivt->boot_data->image_start;
    size_t image_size = ivt->boot_data->image_size;

    /*
    ivt->entry = (hab_image_entry_f)((uintptr_t)ivt->entry - (uintptr_t)ivt->boot_data->image_start + (uintptr_t)base);
    ivt->csf = (hab_csf_t *)((uintptr_t)ivt->csf - (uintptr_t)ivt->boot_data->image_start + (uintptr_t)base);
    ivt->boot_data = (hab_boot_data_t *)((uintptr_t)ivt->boot_data - (uintptr_t)ivt->boot_data->image_start + (uintptr_t)base);
    ivt->self = (void *)((uintptr_t)ivt->self - (uintptr_t)ivt->boot_data->image_start + (uintptr_t)base);
    ivt->boot_data->image_start = base;
    */

    //  Check target boot memory region
    /*
    if (hab_check_target(HAB_TGT_ANY, ivt->boot_data->image_start,
                         ivt->boot_data->image_size) != HAB_SUCCESS)
    {
        printf("Check target failed\n");
        return HAB_FAILURE;
    }
    */

    printf("imgstart %p ivt_offs %i img_size %u\n", image_load_addr, ivt_offset, image_size);
    hab_image_entry_f entry = hab_authenticate_image_no_dcd(0, ivt_offset,
                                                            &image_load_addr, &image_size, NULL);
    printf("XXX HAB entry XXX %p\n", entry);
    // Check the authentication result
    const hab_status_t auth_status = hab_assert(HAB_ASSERT_BLOCK, entry, 0x1000);

    if (entry == NULL || auth_status != HAB_SUCCESS)
    {
        printf("Auth fail entry %p status %02x\n", entry, auth_status);
        HAB_Print_Audit_Log();
        return HAB_FAILURE;
    }

    return HAB_SUCCESS;
}

//! Verify executable binary signature
int sec_verify_executable(const char *path_bin)
{
    if (sec_configuration_is_open())
    {
        printf("Open configuration\n");
        return sec_verify_openconfig;
    }
    uint8_t *binary = load_file_int_sig_space(path_bin);
    if (!binary)
    {
        printf("IOerror errno %i\n", errno);
        return sec_verify_ioerror;
    }
    hab_ivt_t *img_ivt = (hab_ivt_t *)(binary + IVT_OFFSET);
    if (img_ivt->hdr.tag != HAB_TAG_IVT || img_ivt->boot_data == NULL ||
        img_ivt->entry == NULL)
    {
        printf("Invalid evt\n");
        return sec_verify_invalevt;
    }
    Print_Image_Info(img_ivt);

    if (authenticate_image(img_ivt) == HAB_SUCCESS)
    {
        printf("Sec verify sucess\n");
        return sec_verify_ok;
    }
    else
    {
        printf("Sec verify invalid sign\n");
        return sec_verify_invalsign;
    }
    return sec_verify_ok;
}