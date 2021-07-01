#include "test_suite_vfs.h"
#include <seatest/seatest.h>
#include <stdio.h>

/** Tests for the basic read functionality
 * using the FILE stdio access interface
 */
static void test_basic_file_api()
{
    // Open the boot json on master partition
    FILE *file = fopen("/os/.boot.json", "r");
    if (!file)
    {
        assert_fail("Fopen errors skip other tests");
        return;
    }
    assert_int_equal(fclose(file), 0);
}

// VFS test fixutre
void test_fixture_vfs()
{
    test_fixture_start();
    run_test(test_basic_file_api);
    test_fixture_end();
}
