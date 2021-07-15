#include <stdio.h>
#include <common/trace.h>
#include <common/json.h>
#include <boost/test/unit_test.hpp>

#include "checksum_fixture.h"

#define BOOST_TEST_MODULE test json

BOOST_FIXTURE_TEST_CASE(get_json_test, TestChecksum)
{
    trace_list_t tl = trace_init();

    cJSON * json = get_json(&tl, test_json_path.c_str());

    BOOST_TEST(json != nullptr);
    BOOST_TEST(cJSON_GetArraySize(json) == 5);

    cJSON_Delete(json);
    trace_deinit(&tl);
}

BOOST_FIXTURE_TEST_CASE(get_from_json_test, TestChecksum)
{
    trace_list_t tl = trace_init();

    cJSON * json = get_json(&tl, test_json_path.c_str());
    cJSON * group = get_from_json(&tl, json, "git");
    cJSON * item = get_from_json(&tl, group, "git_branch");

    BOOST_TEST(json != nullptr);
    BOOST_TEST(cJSON_GetArraySize(json) == 5);
    BOOST_TEST(group != nullptr);
    BOOST_TEST(cJSON_GetArraySize(group) == 3);
    BOOST_TEST(item != nullptr);
    BOOST_TEST(item->valuestring == "master");

    cJSON_Delete(json);
    trace_deinit(&tl);
}


