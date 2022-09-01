#define BOOST_TEST_MODULE test backup data

#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_log.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/data/monomorphic.hpp>
#include <stdlib.h>
#include "dir_fixture.hpp"
#include "priv_backup.h"


BOOST_FIXTURE_TEST_CASE(fixture_check, OneMDisk)
{
    auto ptr = fopen((drive + "test").c_str(), "a+");
    BOOST_TEST(ptr != nullptr, "we have to be able to create file");

    auto text = std::string();
    text.reserve(meg_size * 1024 * 1024 + 1);
    for (int i = 0; i < meg_size * 1024 * 1024; ++i) {
        text += std::to_string(i%10);
    }

    auto res = fprintf(ptr, "%s",text.c_str()); 
    BOOST_TEST(res == text.length(), "test that we will mimally fit in our tmpfs with files of size: " << meg_size << "mb");
    res = fprintf(ptr, "%s",text.c_str()); 
    BOOST_TEST(res != text.length(), "but want be able to put more than" << meg_size << "mb");

    fclose(ptr);
}

struct wrong_data{
        std::vector<std::pair<const char *, const char *>> paths{
            {nullptr, nullptr},
            {"t1", nullptr},
            {nullptr,"t1"},
            {nullptr, ""},
            {"", nullptr},
            {"", ""},
            {"t1", ""},
            {"", "t1"}
        };
};

/// test for all wrong input paths
BOOST_AUTO_TEST_CASE( backup_entries_check_strings)
{
    auto data = wrong_data();
    for (auto &[p1, p2] : data.paths) {
        struct backup_handle_s h;
        memset(&h, 0, sizeof h);
        h.backup_from_user = p1;
        h.backup_from_os = p1;
        h.backup_to = p2;

        BOOST_TEST(check_backup_entries(&h) == false);
    }
}

/// test for proper input path
BOOST_AUTO_TEST_CASE( backup_entries_check_success)
{
    struct backup_handle_s h;
    memset(&h, 0, sizeof h);
    std::string yay="yay";
    std::string may="may";
    h.backup_from_os = yay.c_str();
    h.backup_from_user = yay.c_str();
    h.backup_to = may.c_str();
    BOOST_TEST(check_backup_entries(&h) == true, "invalid entries");

}
//
/// test for wrong paths with existing partition
BOOST_FIXTURE_TEST_CASE( backup_entries_no_to_directory, AssetEmptyImage)
{
    struct backup_handle_s h;
    memset(&h, 0, sizeof h);
    BOOST_TEST(backup_whole_directory(&h) == false);
}

/// test for success on no job to do
BOOST_FIXTURE_TEST_CASE( backup_entries_success_no_data, AssetEmptyImage)
{
    struct backup_handle_s h;
    memset(&h, 0, sizeof h);
    std::string from =  image.drive;
    std::string to =  disk.drive + "test.tar";
    h.backup_from_os = from.c_str();
    h.backup_from_user = from.c_str();
    h.backup_to = to.c_str();
    BOOST_TEST(backup_whole_directory(&h) == true);
}

/// test for fail with too little space
BOOST_FIXTURE_TEST_CASE( backup_entries_too_small_disk, TooSmall)
{
    struct backup_handle_s h;
    memset(&h, 0, sizeof h);
    std::string from =  image.drive;
    std::string to =  disk.drive + "test.tar";
    h.backup_from_os = from.c_str();
    h.backup_from_user = from.c_str();
    h.backup_to = to.c_str();
    BOOST_TEST(backup_whole_directory(&h) == false);
}

/// test for tafing file - straight in catalog
BOOST_FIXTURE_TEST_CASE( backup_entries_success_sample_data, AverageDisk)
{
    struct backup_handle_s h;
    memset(&h, 0, sizeof h);
    std::string from =  image.drive;
    std::string to =  disk.drive + "test.tar";
    h.backup_from_os = from.c_str();
    h.backup_from_user = from.c_str();
    h.backup_to = to.c_str();
    BOOST_TEST(backup_whole_directory(&h) == true, "cant write data from: "<<from<<" to: " << to );
}


/// test for taring file nested deep into the structure
BOOST_FIXTURE_TEST_CASE( backup_entries_success_deep_sample_data, AverageDiskDeepImage)
{
    struct backup_handle_s h;
    memset(&h, 0, sizeof h);
    std::string from =  image.drive;
    std::string to =  disk.drive + "test.tar";
    h.backup_from_os = from.c_str();
    h.backup_from_user = from.c_str();
    h.backup_to = to.c_str();
    BOOST_TEST(backup_whole_directory(&h) == true, "cant write data from: "<<from<<" to: " << to );
}
