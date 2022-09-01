#include <stdio.h>
#include <common/json.h>
#include <common/json_priv.h>
#include <boost/test/unit_test.hpp>

#include "json_fixture.h"

#define BOOST_TEST_MODULE test json

BOOST_FIXTURE_TEST_CASE(json_get_test, TestsConsts)
{
    cJSON * json = json_get(test_json_path.c_str());

    BOOST_TEST(json != nullptr);
    BOOST_TEST(cJSON_GetArraySize(json) == 5);

    cJSON_Delete(json);
}

BOOST_FIXTURE_TEST_CASE(json_get_item_from_test, TestsConsts)
{
    cJSON * json = json_get(test_json_path.c_str());
    cJSON * group = json_get_item_from(json, "git");
    cJSON * item = json_get_item_from(group, "git_branch");

    BOOST_TEST(json != nullptr);
    BOOST_TEST(cJSON_GetArraySize(json) == 7);
    BOOST_TEST(group != nullptr);
    BOOST_TEST(cJSON_GetArraySize(group) == 3);
    BOOST_TEST(item != nullptr);
    BOOST_TEST(item->valuestring == "master");

    cJSON_Delete(json);
}

BOOST_FIXTURE_TEST_CASE(json_get_version_struct_test, TestsConsts)
{
    version_json_s version_json = json_get_version_struct(test_json_path.c_str());

    BOOST_TEST(strcmp(version_json.boot.name, "boot.bin") == 0);
    BOOST_TEST(strcmp(version_json.boot.md5sum, "123") == 0);
    BOOST_TEST(strcmp(version_json.boot.version, "1.0.12") == 0);

}


