cmake_minimum_required(VERSION 3.2)

project(unittest)
enable_testing()
find_package(Boost REQUIRED COMPONENTS unit_test_framework)

add_executable(
    test_backup
    test_file.cpp
    test_dir_walk.cpp
    test_backup.cpp
    test_update.cpp
    test_checksum.cpp
    dir_fixture.cpp
    test_trace.cpp
    helper.cpp

    ${PROJECT_SOURCE_DIR}/../updater/procedure/backup/backup.c
    ${PROJECT_SOURCE_DIR}/../updater/procedure/backup/priv_backup.c
    ${PROJECT_SOURCE_DIR}/../updater/procedure/backup/dir_walker.c
    ${PROJECT_SOURCE_DIR}/../updater/procedure/package_update/update.c
    ${PROJECT_SOURCE_DIR}/../updater/procedure/package_update/priv_update.c
    ${PROJECT_SOURCE_DIR}/../updater/procedure/checksum/checksum.c
    ${PROJECT_SOURCE_DIR}/../updater/common/trace.c
    ${PROJECT_SOURCE_DIR}/../updater/common/tar.c
    ${PROJECT_SOURCE_DIR}/../updater/common/match.c
    ${PROJECT_SOURCE_DIR}/../updater/common/path_opts.c

    ../third-party/microtar/src/microtar.c
    )

set_property(TARGET test_backup PROPERTY CXX_STANDARD 17)

target_include_directories(
    test_backup
    PRIVATE
    ${PROJECT_SOURCE_DIR}/../updater/
    ${PROJECT_SOURCE_DIR}/../updater/procedure/backup/
    ${Boost_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/../third-party/
    )

target_compile_definitions(test_backup
    PRIVATE
    "BOOST_TEST_DYN_LINK=1"
    BUILD_DIR="${CMAKE_CURRENT_BINARY_DIR}"
    SOURCE_DIR="${CMAKE_SOURCE_DIR}"
    )

target_link_libraries(test_backup ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

add_test(NAME test1 COMMAND test_backup)
