#include <hal/hwcrypt/signature.h>
#include <hal/hwcrypt/sha256.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <drivers/hab/hab.h>
#include <hal/security.h>
#include <stdint.h>
#include <string.h>

//! Maximum allowed file size
#define IVT_OFFSET 0x400
//! Where the SHA is stored
#define SHA_OFFSET 0x1000

//! Temporary signature buffer
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
    const long file_siz = ftell(filp);
    if (fseek(filp, 0, SEEK_SET))
    {
        return NULL;
    }
    if ((size_t)file_siz > sizeof signature)
    {
        errno = -E2BIG;
        return NULL;
    }
    if (fread(signature, file_siz, 1, filp) != 1)
    {
        return NULL;
    }
    return signature;
}

static void hab_print_audit_log(void)
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
                printf("%s: HAB audit log:\n", __PRETTY_FUNCTION__);
            }
            printf("%s: event %lu: status = %s, reason = %s, context = %s\n", __PRETTY_FUNCTION__,
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

    //  Check target boot memory region
    if (hab_check_target(HAB_TGT_MEMORY, ivt->boot_data->image_start,
                         ivt->boot_data->image_size) != HAB_SUCCESS)
    {
        printf("%s: Check target failed\n", __PRETTY_FUNCTION__);
        return HAB_FAILURE;
    }

    hab_image_entry_f entry = hab_authenticate_image_no_dcd(0, ivt_offset,
                                                            &image_load_addr, &image_size, NULL);
    // Check the authentication result
    const hab_status_t auth_status = hab_assert(HAB_ASSERT_BLOCK, entry, 0x1000);
    if (entry == NULL || auth_status != HAB_SUCCESS)
    {
        printf("%s: Auth fail entry %p status %02x\n", __PRETTY_FUNCTION__, entry, auth_status);
        hab_print_audit_log();
        return HAB_FAILURE;
    }
    return HAB_SUCCESS;
}

//! Verify executable binary signature
static int verify_sig_bin_blob(const char *path_bin, uint8_t **buffer)
{
    if (sec_configuration_is_open())
    {
        printf("%s: Configuration is open unable to verify signature\n", __PRETTY_FUNCTION__);
        return sec_verify_openconfig;
    }
    uint8_t *binary = load_file_int_sig_space(path_bin);
    if (!binary)
    {
        printf("%s: Unable to load file %s\n", __PRETTY_FUNCTION__, path_bin);
        return sec_verify_ioerror;
    }
    hab_ivt_t *img_ivt = (hab_ivt_t *)(binary + IVT_OFFSET);
    if (img_ivt->hdr.tag != HAB_TAG_IVT || img_ivt->boot_data == NULL ||
        img_ivt->entry == NULL)
    {
        printf("%s: Invalid evt vector in signature\n", __PRETTY_FUNCTION__);
        return sec_verify_invalevt;
    }

    if (authenticate_image(img_ivt) == HAB_SUCCESS)
    {
        if (buffer)
        {
            *buffer = binary;
        }
        return sec_verify_ok;
    }
    else
    {
        return sec_verify_invalsign;
    }
}

//! Verify signature for the file
int sec_verify_file(const char *file, const char *signature_file)
{
    struct sha256_hash sha;
    int err = sha256_file(file, &sha);
    if (err)
    {
        errno = -err;
        printf("%s: Unable to calculate checksum errno %i\n", __PRETTY_FUNCTION__, -err);
        return sec_verify_ioerror;
    }
    uint8_t *buf;
    err = verify_sig_bin_blob(signature_file, &buf);
    if (err)
    {
        return err;
    }
    if (memcmp(buf + SHA_OFFSET, sha.value, sizeof sha.value) == 0)
    {
        return sec_verify_ok;
    }
    else
    {
        printf("%s: SHA mismatch in the signature\n", __PRETTY_FUNCTION__);
        sha256_print_hash("Calculated hash", &sha);
        sha256_print_hash("Signature hash", (struct sha256_hash *)(buf + SHA_OFFSET));
        return sec_verify_invalid_sha;
    }
}
