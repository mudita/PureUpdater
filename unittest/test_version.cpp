#include <stdio.h>
#include <common/trace.h>
#include <common/json.h>
#include <boost/test/unit_test.hpp>
#define BOOST_TEST_MODULE test version
#include "json_fixture.h"
#include "version.h"
#include "version_priv.h"

BOOST_AUTO_TEST_CASE(parse_version_str_test)
{
    trace_list_t tl = trace_init();

    const char * test_ver_string = "0.72.1";
    version_s version;

    int ret = version_parse_str(&tl, &version, test_ver_string);

    BOOST_TEST(ret == 0);
    BOOST_TEST(version.major == 0);
    BOOST_TEST(version.minor == 72);
    BOOST_TEST(version.patch == 1);
    trace_deinit(&tl);
}

BOOST_FIXTURE_TEST_CASE(get_version_test, TestsConsts)
{
    trace_list_t tl = trace_init();

    version_json_s version_json = json_get_version_struct(&tl, test_json_path.c_str());
    version_s version;
    version_parse_str(&tl, &version, version_json.boot.version);

    BOOST_TEST(version.major == 1);
    BOOST_TEST(version.minor == 0);
    BOOST_TEST(version.patch == 12);
    trace_deinit(&tl);
}

BOOST_AUTO_TEST_CASE(version_is_lhs_newer_test)
{
    trace_list_t tl = trace_init();

    version_s version = { .major = 1, .minor = 99, .patch =24, .valid = true};
    version_s version2 = { .major = 1, .minor = 99, .patch =25, .valid = true};

    BOOST_TEST(version_is_lhs_newer(&version2, &version));

    version2.patch = 23;

    BOOST_TEST(!version_is_lhs_newer(&version2, &version));

    version2.patch = 24;
    version2.minor = 98;

    BOOST_TEST(!version_is_lhs_newer(&version2, &version));

    version2.minor = 99;
    version2.major = 2;

    BOOST_TEST(version_is_lhs_newer(&version2, &version));
    trace_deinit(&tl);
}

BOOST_AUTO_TEST_CASE(get_version_str_test)
{
    trace_list_t tl = trace_init();

    const char * test_str = "1.99.24";
    version_s version = { .major = 1, .minor = 99, .patch =24, .valid = true};

    const char * ret = version_get_str(&version);

    BOOST_TEST(strcmp(ret, test_str) == 0);
    BOOST_TEST(strcmp(ret, version.str) == 0);
    trace_deinit(&tl);
}
