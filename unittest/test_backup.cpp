#include <boost/process/io.hpp>
#include <boost/process/system.hpp>
#include <boost/test/unit_test.hpp>
#define BOOST_TEST_MODULE test backup boot partition
#include "helper.hpp"
#include "dir_fixture.hpp"
#include "priv_backup.h"

BOOST_FIXTURE_TEST_CASE(backup_success, Firmware)
{
    struct backup_handle_s h {0,0};

    std::string from =  image.drive;
    std::string end_tar =  disk.drive + "test.tar";
    h.backup_from_os = from.c_str();
    h.backup_from_user = from.c_str();
    h.backup_to = end_tar.c_str();

    BOOST_TEST(backup_boot_partition(&h) == true, "we can write data from: "<<from<<" to: " << end_tar );
    BOOST_TEST(backup_user_data(&h) == true, "we can append data from: "<<from<<" to: " << end_tar );
    BOOST_TEST(std::filesystem::exists(end_tar));

    unpack(end_tar, disk.drive);
    BOOST_TEST(std::filesystem::exists(disk.drive + "boot.bin"));
    BOOST_TEST(std::filesystem::exists(disk.drive + "version.json"));
    BOOST_TEST(std::filesystem::exists(disk.drive + "country-codes.db"));
}
