#include "common/trace.h"
#include "update.h"
#include <boost/process/io.hpp>
#include <boost/process/system.hpp>
#include <boost/test/unit_test.hpp>
#include <filesystem>
#define BOOST_TEST_MODULE test update
#include "priv_tmp.h"
#include "dir_fixture.hpp"

/// this test wont work fill catalogs will work
BOOST_FIXTURE_TEST_CASE(test_tmp_success, UpdateAsset)
{
   trace_list_t tl = trace_init();

    std::string in = image.tar_path.string() + image.tar_name;
    std::string os_tmp = disk_os.drive + "/tmp_os";
    std::string user_tmp = disk_os.drive + "/tmp_user";


    update_handle_s handle = {};
    update_firmware_init(&handle);
    handle.update_from = in.c_str();
    handle.update_os   = disk_os.drive.c_str();
    handle.update_user = disk_user.drive.c_str();
    handle.tmp_os      = os_tmp.c_str();
    handle.tmp_user    = user_tmp.c_str();

    BOOST_TEST(create_temp_catalog(&handle,&tl) == true);

    BOOST_TEST(std::filesystem::exists(os_tmp), "OS tmp exists: " << os_tmp);
    BOOST_TEST(std::filesystem::exists(user_tmp), "USER tmp exists: " << user_tmp);

    BOOST_TEST(create_temp_catalog(&handle,&tl) == true);

    std::filesystem::copy(in, os_tmp);
    std::filesystem::create_directory(os_tmp + "/lol/");
    std::filesystem::copy(in, os_tmp + "/lol/test.tar");
    std::filesystem::create_directory(user_tmp + "/lol2");

    BOOST_TEST(create_temp_catalog(&handle,&tl) == true);
    std::filesystem::copy(in, os_tmp);
    std::filesystem::create_directory(os_tmp + "/lol/");
    std::filesystem::copy(in, os_tmp + "/lol/test.tar");
    std::filesystem::create_directory(user_tmp + "/lol2");

    BOOST_TEST(files_move(&handle, &tl) == true);

    BOOST_TEST(std::filesystem::exists(disk_os.drive + "/lol/test.tar"));
    BOOST_TEST(std::filesystem::exists(disk_user.drive + "/lol2"));

    /// move empty data
    BOOST_TEST(files_move(&handle, &tl) == true);

    if (!trace_list_ok(&tl))
       trace_print(&tl);

    trace_deinit(&tl);
}
