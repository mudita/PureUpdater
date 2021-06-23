#include "dir_walker.h"
#include <boost/test/unit_test.hpp>
#define BOOST_TEST_MODULE test module dir walk
#include "dir_walker.h"
#include "dir_fixture.hpp"

auto counter = [](const char *, dir_handling_type_e what,dir_handler_s *h, void* data) -> int
{
    (void)what;
    (void)h;
    ++(*(int*)data);
    return 0;
};

BOOST_FIXTURE_TEST_CASE( walker_nothing, OneMDisk)
{
    int cnt = 0;
    dir_handler_s handle;
    recursive_dir_walker_init(&handle, counter, &cnt);
    unsigned int depth = 10;
    recursive_dir_walker(drive.c_str(), &handle, &depth);
    recursive_dir_walker_deinit(&handle);
    BOOST_TEST(0 == handle.error);
}


BOOST_FIXTURE_TEST_CASE( walker_one_file, StandardImage)
{
    int cnt=0;
    unsigned int depth = 10;
    dir_handler_s handle;
    recursive_dir_walker_init(&handle, counter, &cnt);
    recursive_dir_walker(drive.c_str(), &handle, &depth);
    recursive_dir_walker_deinit(&handle);
    BOOST_TEST(1 == cnt);
    BOOST_TEST(0 == handle.error);
}

struct getter {
    int cnt =0;
    std::string path;
};

auto pather = [](const char *name, dir_handling_type_e what,dir_handler_s *h, void* data) -> int
{
    (void)what;
    (void)h;
    auto g = static_cast<getter*>(data);
    g->path = name;
    ++g->cnt;
    return 0;
};

BOOST_FIXTURE_TEST_CASE(walker_recursion, DeepImage)
{
    getter getme;
    dir_handler_s handle;
    unsigned int depth = 1;
    recursive_dir_walker_init(&handle, pather, &getme);
    recursive_dir_walker(drive.c_str(), &handle, &depth);
    recursive_dir_walker_deinit(&handle);
    BOOST_TEST(0 == depth, "We should have hit the bottom " << depth);
    BOOST_TEST(DirHandlingRecursionLimit == handle.error,
               "Error: " << dir_handling_strerror(handle.error)
                         << " but should be: " << dir_handling_strerror(DirHandlingRecursionLimit));
}

BOOST_FIXTURE_TEST_CASE(walker_in_depth, DeepImage)
{
    getter getme;
    dir_handler_s handle;
    unsigned int depth = 10;
    recursive_dir_walker_init(&handle, pather, &getme);
    recursive_dir_walker(drive.c_str(), &handle, &depth);
    recursive_dir_walker_deinit(&handle);
    BOOST_TEST(0 != getme.cnt);
    BOOST_TEST(0 == handle.error, "Error: " << dir_handling_strerror(handle.error));
    BOOST_TEST(deep_path + tar_name == getme.path, "full path from . is: " << getme.path);
}
