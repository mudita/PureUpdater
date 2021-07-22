#include <hal/hwcrypt/sha256.h>
#include <drivers/fsl_dcp.h>
#include <errno.h>
#include <stdio.h>

// Bitmap for store used Crypto channels per hash
static dcp_channel_t g_used_channel = 0;

// Note HW context must be in the OC RAM
#define DCP_NUM_CHANNELS 4

static __attribute__((section(".intramnoncacheable"))) dcp_hash_ctx_t g_dcp_hash_ctx[DCP_NUM_CHANNELS];

// Internal context
struct sha256_context
{
    dcp_hash_ctx_t *ctx;
    dcp_handle_t handle;
};

/** Allocate DCP channel for hash
 * NOTE: Start from kDCP_Channel1 kDCP_Channel0 is reserved by the HAB
 */
static dcp_channel_t alloc_channel(void)
{
    for (uint32_t chn = kDCP_Channel1; chn <= kDCP_Channel3; chn <<= 1U)
    {
        if ((g_used_channel & chn) == 0)
        {
            g_used_channel |= chn;
            return chn;
        }
    }
    return 0;
}

// Free DCP channel
static int free_channel(dcp_channel_t chn)
{
    if ((g_used_channel & chn) == 0)
    {
        return -ENOENT;
    }
    g_used_channel &= ~chn;
    return 0;
}

static dcp_hash_ctx_t *get_hash_buf(unsigned chn)
{
    if (chn < kDCP_Channel0 || chn > kDCP_Channel3)
    {
        return NULL;
    }
    chn = __builtin_ffs((int)chn) - __builtin_ffs(kDCP_Channel0);
    return &g_dcp_hash_ctx[chn];
}

// Initialize SHA 256 context
struct sha256_context *sha256_init(void)
{
    dcp_channel_t chn = alloc_channel();
    if (!chn)
    {
        errno = EMFILE;
        return NULL;
    }
    struct sha256_context *ctx = calloc(1, sizeof(struct sha256_context));
    if (!ctx)
    {
        free_channel(chn);
        return ctx;
    }
    ctx->handle.channel = chn;
    ctx->ctx = get_hash_buf(chn);
    if (!ctx->ctx)
    {
        errno = EINVAL;
        free(ctx);
        free_channel(chn);
        ctx = NULL;
    }
    if (DCP_HASH_Init(DCP, &ctx->handle, ctx->ctx, kDCP_Sha256) != kStatus_Success)
    {
        errno = EINVAL;
        free(ctx);
        free_channel(chn);
        ctx = NULL;
    }
    return ctx;
}

// Udate SHA 256 hash
int sha256_update(struct sha256_context *ctx, const void *data, size_t size)
{
    if (!ctx)
    {
        return -EINVAL;
    }
    if (!data)
    {
        return -EINVAL;
    }
    int err;
    if ((err = DCP_HASH_Update(DCP, ctx->ctx, data, size)) != kStatus_Success)
    {
        return -EIO;
    }
    return 0;
}

// Finish the hash
int sha256_finish(struct sha256_context *ctx, struct sha256_hash *hash)
{
    if (!ctx)
    {
        return -EINVAL;
    }
    if (!hash)
    {
        return -EINVAL;
    }
    int err = 0;
    size_t outputsz = sizeof(hash->value);
    if (DCP_HASH_Finish(DCP, ctx->ctx, hash->value, &outputsz) != kStatus_Success)
    {
        err = -EIO;
    }
    if (outputsz != sizeof(hash->value))
    {
        err = -EOVERFLOW;
    }
    err = free_channel(ctx->handle.channel);
    free(ctx);
    return err;
}

//! Cleanup file descriptor
static void file_clean_up(FILE **fil)
{
    if (*fil)
    {
        fclose(*fil);
    }
}

static void free_clean_up(uint8_t **ptr)
{
    free(*ptr);
}

static void sha_clean_up(struct sha256_context **ctx)
{
    if (*ctx)
    {
        struct sha256_hash hash;
        sha256_finish(*ctx, &hash);
    }
}

// SHA256 on file calculate
int sha256_file(const char *path, struct sha256_hash *hash)
{
    if (!path)
    {
        return -EINVAL;
    }
    if (!hash)
    {
        return -EINVAL;
    }
    FILE *filp __attribute__((__cleanup__(file_clean_up))) = fopen(path, "rb");
    if (!filp)
    {
        return -errno;
    }
    static const size_t buf_size = 16384;
    uint8_t *buf __attribute__((__cleanup__(free_clean_up))) = malloc(buf_size);
    if (!buf)
    {
        return -errno;
    }
    if (fseek(filp, 0, SEEK_END))
    {
        return -errno;
    }
    const long fil_siz = ftell(filp);
    if (fseek(filp, 0, SEEK_SET))
    {
        return -errno;
    }
    if (fil_siz < 0)
    {
        return fil_siz;
    }
    struct sha256_context *ctx __attribute__((__cleanup__(sha_clean_up))) = sha256_init();
    if (!ctx)
    {
        return -errno;
    }
    int ret;
    for (size_t sz = fil_siz; sz > 0;)
    {
        if (sz > buf_size)
        {
            if (fread(buf, buf_size, 1, filp) != 1)
            {
                return -errno;
            }
            sz -= buf_size;
            if ((ret = sha256_update(ctx, buf, buf_size)))
            {
                return ret;
            }
        }
        else
        {
            if (fread(buf, sz, 1, filp) != 1)
            {
                return -errno;
            }
            if ((ret = sha256_update(ctx, buf, sz)))
            {
                return ret;
            }
            sz -= sz;
        }
    }
    ret = sha256_finish(ctx, hash);
    ctx = NULL;
    return ret;
}

// SHA 256 for the memory buffer
int sha256_mem(const void *buf, size_t len, struct sha256_hash *hash)
{
    int ret;
    if (!buf)
    {
        return -EINVAL;
    }
    if (!hash)
    {
        return -EINVAL;
    }
    if (len <= 0)
    {
        return -EINVAL;
    }
    struct sha256_context *ctx __attribute__((__cleanup__(sha_clean_up))) = sha256_init();
    if (!ctx)
    {
        return -errno;
    }
    if ((ret = sha256_update(ctx, buf, len)))
    {
        return ret;
    }
    ret = sha256_finish(ctx, hash);
    ctx = NULL;
    return ret;
}