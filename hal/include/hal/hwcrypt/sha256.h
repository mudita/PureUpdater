#pragma once

#include <stdint.h>
#include <stddef.h>

//! Sha 256 hash
struct sha256_hash
{
    uint8_t value[32];
};

//!Internal context of the SHA256
struct sha256_context;

/** Initialize SHA 256 context
 * @return SHA internal context
 */
struct sha256_context *sha256_init(void);

/** Update SHA256 hash value
 * @param[in] ctx Hash context
 * @param[in] data Data for update
 * @param[in] size Data size
 * @return 0 on success -errno on failure
 */
int sha256_update(struct sha256_context *ctx, const void *data, size_t size);

/** Finish sha256hash and free the context resources
 * @param[in] ctx Hash context
 * @param[out] hash Output sha256 hash
 * @return 0 on sucess -errno on failure
 */
int sha256_finish(struct sha256_context *ctx, struct sha256_hash *hash);

/** Calculate sha256 for the file 
 * @param[in] path Path for file
 * @param[out] hash Output sha256 hash
 * @return 0 on sucess -errno on failure
 */
int sha256_file(const char *path, struct sha256_hash *hash);

/** Calculate sha256 for the memory buffer
 * @param[in] buf Pointer to memory buffer
 * @param[in] len Buffer length
 * @param[out] hash Output sha256 hash
 * @return 0 on sucess -errno on failure
 */
int sha256_mem(const void *buf, size_t len, struct sha256_hash *hash);
