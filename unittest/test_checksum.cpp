#include "common/trace.h"
#include <boost/test/unit_test.hpp>
#define BOOST_TEST_MODULE test checksum
#include "checksum.h"

// BOOST_FIXTURE_TEST_CASE(success_checksum, Tutaj wstaw fixture jak chcesz)
BOOST_AUTO_TEST_CASE(checksum_success)
{
    trace_list_t tl = trace_init();
    struct backup_handle_s backup_handle;
    BOOST_TEST(checksum_verify(&backup_handle, &tl), "TODO: " << "implement me");
    trace_deinit(&tl);
}
