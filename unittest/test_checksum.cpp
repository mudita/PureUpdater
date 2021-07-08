#include <stdio.h>
#include <common/trace.h>
#include <boost/test/unit_test.hpp>
#include <cJSON/cJSON.h>
#include <md5/md5.h>
#define BOOST_TEST_MODULE test checksum
#include "checksum.h"
#include "checksum_fixture.h"


BOOST_FIXTURE_TEST_CASE(get_json_test, TestChecksum)
{
    trace_list_t tl = trace_init();

    cJSON * json = get_json(&tl, test_json_path.c_str());

    BOOST_TEST(json != nullptr);
    BOOST_TEST(cJSON_GetArraySize(json) == 5);

    cJSON_Delete(json);
    trace_deinit(&tl);
}

BOOST_FIXTURE_TEST_CASE(get_checksum_test, TestChecksum)
{
    trace_list_t tl = trace_init();

    const cJSON * json = cJSON_Parse(test_json.c_str());

    const char * checksum = get_checksum(&tl, json, "boot.bin");

    BOOST_TEST(strcmp(checksum, mock_checksum_boot.c_str()) == 0);
    trace_deinit(&tl);
}

BOOST_FIXTURE_TEST_CASE(compare_checksum, TestChecksum)
{
    BOOST_TEST(compare_checksums(mock_checksum_boot.c_str(),mock_checksum_boot.c_str()) == true);
    BOOST_TEST(compare_checksums(mock_checksum_boot.c_str(),mock_checksum_ecoboot.c_str()) == false);
}

BOOST_FIXTURE_TEST_CASE(md5_calculate_checksum, TestChecksum)
{
    unsigned char checksum[16];

    MD5_File(checksum, test_json_path.c_str());

    // Convert hex output to readable string for test purpose
    char checksum_readable[33];
    char *ptr = &checksum_readable[0];
    for(size_t i = 0; i < 16; ++i){
        ptr += sprintf(ptr, "%02X", checksum[i]);
    }

    BOOST_TEST(memcmp(checksum_readable, test_json_md5_checksum, 16) == 0);
}

