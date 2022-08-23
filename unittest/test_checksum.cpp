#include <stdio.h>
#include <common/json.h>
#include <boost/test/unit_test.hpp>
#include <md5/md5.h>
#define BOOST_TEST_MODULE test checksum
#include "checksum.h"
#include "checksum_priv.h"
#include "json_fixture.h"

BOOST_FIXTURE_TEST_CASE(checksum_verify_test, TestsConsts)
{
    verify_file_handle_s handle;
    handle.file_to_verify = test_checksum_file_path.c_str();
    handle.version_json = json_get_version_struct(test_json_path.c_str());

    BOOST_TEST(checksum_verify(&handle));
}

BOOST_FIXTURE_TEST_CASE(checksum_compare_test, TestsConsts)
{
    BOOST_TEST(checksum_compare(mock_checksum1.c_str(),mock_checksum1.c_str()) == true);
    BOOST_TEST(checksum_compare(mock_checksum1.c_str(),mock_checksum2.c_str()) == false);
}

BOOST_FIXTURE_TEST_CASE(md5_calculate_checksum, TestsConsts)
{
    unsigned char checksum[16];

    MD5_File(checksum, test_checksum_file_path.c_str());

    // Convert hex output to readable string for test purpose
    char checksum_readable[33];
    char *ptr = &checksum_readable[0];
    for(size_t i = 0; i < 16; ++i){
        ptr += sprintf(ptr, "%02X", checksum[i]);
    }

    BOOST_TEST(memcmp(checksum_readable, test_checksum_file_path_md5, 32) == 0);
}

