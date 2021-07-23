#include "test_suite_crypto.h"
#include <seatest/seatest.h>
#include <hal/hwcrypt/sha256.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

//! Test crypto for invalid args
static void test_sha_invargs(void)
{
    struct sha256_hash hash;
    char buf[64];
    assert_int_equal(-EINVAL, sha256_file(NULL, &hash));
    assert_int_equal(-EINVAL, sha256_file("xxx", NULL));
    assert_int_equal(-ENOENT, sha256_file("/os/non_existient", &hash));
    assert_int_equal(-EINVAL, sha256_mem(NULL, 5, &hash));
    assert_int_equal(-EINVAL, sha256_mem(&buf, 0, &hash));
    assert_int_equal(-EINVAL, sha256_mem(&buf, sizeof buf, NULL));
}

//! Tests for the standard handling
static void test_standard_ops(void)
{
    static const struct sha256_hash expected = {.value = {0x4f, 0xe7, 0xb5, 0x9a, 0xf6, 0xde, 0x3b, 0x66, 0x5b,
                                                          0x67, 0x78, 0x8c, 0xc2, 0xf9, 0x98, 0x92, 0xab, 0x82,
                                                          0x7e, 0xfa, 0xe3, 0xa4, 0x67, 0x34, 0x2b, 0x3b, 0xb4,
                                                          0xe3, 0xbc, 0x8e, 0x5b, 0xfe}};
    static const char *test_fname = "/user/zero16";
    static const size_t file_len = 16 * 1024;
    char buf[16 * 1024] = {};
    struct sha256_hash hash;
    // Hash memory
    assert_int_equal(0, sha256_mem(buf, sizeof buf, &hash));
    assert_n_array_equal(hash.value, expected.value, sizeof expected.value);
    // Hash file (create zero file)
    assert_int_equal(0, truncate(test_fname, file_len));
    memset(&hash, 0, sizeof hash);
    assert_int_equal(0, sha256_file(test_fname, &hash));
    assert_n_array_equal(hash.value, expected.value, sizeof expected.value);
    assert_int_equal(0, unlink(test_fname));
}

//! Test fixtures for crypto
void test_suite_crypto(void)
{
    test_fixture_start();
    test_sha_invargs();
    test_fixture_end();
}