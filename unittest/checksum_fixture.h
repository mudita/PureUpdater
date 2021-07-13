#include <string>
#include <type_traits>
#include <concepts>
#include <filesystem>
#include <stdlib.h>

#include "checksum.h"

#ifndef SOURCE_DIR
#error Requires top dir to get assets
#endif

struct TestChecksum
{
    struct checksum_handle_s checksum_handle;
    const std::string test_json_path {std::string(SOURCE_DIR) + "/assets/test_version.json"};
    const std::string mock_checksum_boot = "0123456789abcdef";
    const std::string mock_checksum_ecoboot = "123456789abcdef0";
    const unsigned char test_json_md5_checksum[33] = "911EA6B3FFAB582651D82AC8A7CA3B65";
    const std::string test_json = "{\n"
                            "\t\"git\":\n"
                            "\t{\n"
                            "\t\t\"git_branch\": \"master\",\n"
                            "\t\t\"git_commit\": \"049147d77\",\n"
                            "\t\t\"git_tag\": \"release-0.72.1-rc1-8-g049147d77\"\n"
                            "\t},\n"
                            "\t\"misc\":\n"
                            "\t{\n"
                            "\t\t\"codename\": \"\",\n"
                            "\t\t\"kernel\": \"V10.2.0\",\n"
                            "\t\t\"buildon\": \"5.11.0-22-generic\",\n"
                            "\t\t\"builddate\": \"2021-07-02-13:28:59\",\n"
                            "\t\t\"builduser\": \"Maciej Janicki\"\n"
                            "\t},\n"
                            "\t\"version\":\n"
                            "\t{\n"
                            "\t\t\"major\": \"0\",\n"
                            "\t\t\"minor\": \"72\",\n"
                            "\t\t\"patch\": \"1\",\n"
                            "\t\t\"string\": \"0.72.1\"\n"
                            "\t},\n"
                            "\t\"bootloader\":\n"
                            "\t{\n"
                            "\t\t\"included\": \"true\",\n"
                            "\t\t\"version\": \"1.0.11\",\n"
                            "\t\t\"filename\": \"ecoboot.bin\"\n"
                            "\t},\n"
                            "\t\"checksums\":\n"
                            "\t{\n"
                            "\t\t\"boot.bin\": \"0123456789abcdef\",\n"
                            "\t\t\"ecoboot.bin\": \"123456789abcdef0\"\n"
                            "\t}\n"
                            "}";
};
