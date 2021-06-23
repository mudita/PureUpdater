#include "dir_fixture.hpp"
#include <boost/test/unit_test.hpp>
#include <boost/process/system.hpp>
#include <boost/process/io.hpp>
#include <boost/process/pipe.hpp>
// #define BOOST_TEST_MODULE test module fixture


Disc::Disc(int meg_size) : meg_size(meg_size)
{
    BOOST_TEST_MESSAGE("setup fixture");
    boost::process::ipstream is;
    auto code = boost::process::system("mktemp -d " BUILD_DIR "/testdrive.XXXX", boost::process::std_out > is);
    BOOST_ASSERT(code == 0);
    is >> drive;
    BOOST_ASSERT(!drive.empty());
    drive += "/";
    code =
        boost::process::system("sudo mount -t tmpfs -o size=" + std::to_string(meg_size) + "m test-ramdisk " + drive);
    BOOST_ASSERT(code == 0);
    BOOST_TEST_MESSAGE("tmp_fs:" << drive);
}

Disc::~Disc()
{
    BOOST_ASSERT(umount() == 0);
    BOOST_ASSERT(clean_dir() == 0);
    BOOST_TEST_MESSAGE("teardown tmp_fs: " << drive << " done!");
}

int Disc::umount()
{
    BOOST_ASSERT(!drive.empty());
    return boost::process::system("sudo umount " + drive);
}

int Disc::clean_dir()
{
    BOOST_ASSERT(!drive.empty());
    return boost::process::system("rm -fdr " + drive);
}

EmptyImage::EmptyImage()
{
    BOOST_TEST_MESSAGE("setup fixture" << "tar: " << tar_path.string() + tar_name);
    BOOST_ASSERT(std::filesystem::exists(tar_path.string() + tar_name));
    boost::process::ipstream is;
    auto code = boost::process::system("mktemp -d " BUILD_DIR "/from.XXXX", boost::process::std_out > is);
    BOOST_ASSERT(code == 0);
    is >> drive;
    BOOST_ASSERT(not drive.empty());
    drive += "/";
    BOOST_ASSERT(code == 0);
}

EmptyImage::~EmptyImage()
{
    auto code = boost::process::system("rm -fdr " + drive);
    BOOST_ASSERT(code == 0);
}

StandardImage::StandardImage()
{
    auto code = boost::process::system("cp " + tar_path.string() + tar_name + " " + drive);
    BOOST_ASSERT(code == 0);
    BOOST_TEST_MESSAGE("tmp drive:" << drive);
}

UnpackedImage::UnpackedImage()
{
    boost::process::ipstream is;
    auto code = boost::process::system("tar -xvf " + tar_path.string() + tar_name + " -C " + drive, boost::process::std_out > is);
    BOOST_ASSERT(code == 0);
}

DeepImage::DeepImage()
{
    deep_path = drive + "/some/deep/path/";
    auto code = boost::process::system("mkdir " + deep_path + " -p");
    BOOST_ASSERT(code == 0);
    code = boost::process::system("cp " + tar_path.string() + tar_name + " " + deep_path);
    BOOST_ASSERT(code == 0);
    BOOST_TEST_MESSAGE("tmp drive:" << drive);
}
