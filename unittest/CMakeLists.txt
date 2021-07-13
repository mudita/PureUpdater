cmake_minimum_required(VERSION 3.2)

enable_testing()
message("compiler set to  ${CMAKE_C_COMPILER}")
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

    ${SRC_FILES}
    ${PROJECT_SOURCE_DIR}/updater/procedure/backup/backup.c
    ${PROJECT_SOURCE_DIR}/updater/procedure/backup/priv_backup.c
    ${PROJECT_SOURCE_DIR}/updater/procedure/backup/dir_walker.c
    ${PROJECT_SOURCE_DIR}/updater/procedure/package_update/update.c
    ${PROJECT_SOURCE_DIR}/updater/procedure/package_update/priv_update.c
    ${PROJECT_SOURCE_DIR}/updater/procedure/checksum/checksum.c
    )

target_compile_options( test_backup PRIVATE -Wall -Wextra)

set_property(TARGET test_backup PROPERTY CXX_STANDARD 17)

target_include_directories(
    test_backup
    PRIVATE
    ${Boost_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/updater/procedure/backup/
    ${PROJECT_SOURCE_DIR}/updater/procedure/package_update/
    ${PROJECT_SOURCE_DIR}/updater/proazienka, tylko 2,5 m2, czy jest szansa na prysznic?cedure/checksum/
    )

target_compile_definitions(test_backup
    PRIVATE
    "BOOST_TEST_DYN_LINK=1"
    BUILD_DIR="${CMAKE_CURRENT_BINARY_DIR}"
    SOURCE_DIR="${CMAKE_CURRENT_LIST_DIR}"
    )

target_link_libraries(test_backup ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    klib
    common
    cjson
    md5
    )

add_test(NAME test1 COMMAND test_backup)

