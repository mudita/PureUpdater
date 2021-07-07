#include "common/trace.h"
#include <boost/process/io.hpp>
#include <boost/process/system.hpp>
#include <boost/test/unit_test.hpp>
#define BOOST_TEST_MODULE test update
#include "helper.hpp"
#include "dir_fixture.hpp"
#include "priv_update.h"

/// this test wont work fill catalogs will work
BOOST_FIXTURE_TEST_CASE(unpack_success, UpdateAsset)
{
    std::string in = image.tar_path.string() + image.tar_name;
    struct update_handle_s handle{0,0,0,0};
    handle.update_from = in.c_str();
    handle.update_os = disk_os.drive.c_str();
    handle.update_user = disk_user.drive.c_str();

    trace_list_t trace_list = trace_init();
    BOOST_TEST(unpack(&handle, &trace_list));
    trace_print(&trace_list);
 

    BOOST_TEST(std::filesystem::exists(disk_os.drive + "boot.bin"));
    BOOST_TEST(std::filesystem::exists(disk_os.drive + "version.json"));
    BOOST_TEST(std::filesystem::exists(disk_user.drive + "country-codes.db"));
    BOOST_TEST(std::filesystem::exists(disk_user.drive + "assets/audio/sms/sms_guitar_5.mp3"));
 
    trace_deinit(&trace_list);
}


BOOST_FIXTURE_TEST_CASE(unpack_no_tar, UpdateEmptyAsset)
{
    std::string in = image.tar_path.string() + image.tar_name;
    struct update_handle_s handle{0,0,0,0};
    handle.update_from = in.c_str();
    handle.update_os = disk_os.drive.c_str();
    handle.update_user = disk_user.drive.c_str();

    trace_list_t trace_list = trace_init();
    BOOST_TEST(unpack(&handle, &trace_list));
    trace_print(&trace_list);
 
    trace_deinit(&trace_list);
}

// // TODO
// BOOST_FIXTURE_TEST_CASE(unpack_error_small, UpdateEmptyAsset)
// {
//     std::string in = image.tar_path.string() + image.tar_name;
//     struct update_handle_s handle;
//     handle.update_from = in.c_str();
//     handle.update_os = disk_os.drive.c_str();
//     handle.update_user = disk_user.drive.c_str();
//
//     trace_list_t trace_list = trace_init();
//     BOOST_TEST(!unpack(&handle, &trace_list));
//     BOOST_TEST(!trace_list_ok(&trace_list));
//     trace_print(&trace_list);
//
//     trace_deinit(&trace_list);
// }
// BOOST_FIXTURE_TEST_CASE(unpack_fs_issue, UpdateAsset)
// BOOST_FIXTURE_TEST_CASE(unpack_overwrite_success, UpdateAsset)
// BOOST_FIXTURE_TEST_CASE(version_check, UpdateAsset)
// BOOST_FIXTURE_TEST_CASE(checksum_check, UpdateAsset)
// BOOST_FIXTURE_TEST_CASE(data_move, UpdateAsset)
