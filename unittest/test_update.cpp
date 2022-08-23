#include "priv_tmp.h"
#include "update.h"
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
    struct update_handle_s handle;
    update_firmware_init(&handle);
    handle.update_from = in.c_str();
    handle.update_os   = disk_os.drive.c_str();
    handle.update_user = disk_user.drive.c_str();
    handle.tmp_os      = (disk_os.drive + "/tmp").c_str();
    handle.tmp_user    = (disk_user.drive + "/tmp").c_str();

    create_temp_catalog(&handle);
    BOOST_TEST(unpack(&handle));


    BOOST_TEST(std::filesystem::exists(disk_os.drive + "boot.bin"));
    BOOST_TEST(std::filesystem::exists(disk_os.drive + "version.json"));
    BOOST_TEST(std::filesystem::exists(disk_user.drive + "country-codes.db"));
    BOOST_TEST(std::filesystem::exists(disk_user.drive + "assets/audio/sms/sms_guitar_5.mp3"));
 
}


BOOST_FIXTURE_TEST_CASE(unpack_no_tar, UpdateEmptyAsset)
{
    std::string in = image.tar_path.string() + image.tar_name;
    struct update_handle_s handle;
    update_firmware_init(&handle);
    handle.update_from = in.c_str();
    handle.update_os   = disk_os.drive.c_str();
    handle.update_user = disk_user.drive.c_str();
    handle.tmp_os      = (disk_os.drive + "/tmp").c_str();
    handle.tmp_user    = (disk_user.drive + "/tmp").c_str();


    create_temp_catalog(&handle);
    BOOST_TEST(unpack(&handle));

}