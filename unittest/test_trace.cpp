#include <boost/process/io.hpp>
#include <boost/process/system.hpp>
#include <boost/test/unit_test.hpp>
#define BOOST_TEST_MODULE test tracing
#include "common/trace.h"

const char *lol(int i)
{
    (void)i;
    return "lol";
}

const char *troll(int i)
{
    (void)i;
    return "troll";
}

BOOST_AUTO_TEST_CASE(test_trace)
{
    trace_list_t traces = trace_init();
    trace_t *t0 = trace_append("trace 0", &traces, NULL, NULL);
    trace_t *t1 = trace_append("trace 1", &traces, lol, NULL);
    trace_t *t2 = trace_append("trace 2", &traces, troll, NULL);

    trace_write(t0, 0, 0);
    BOOST_TEST(trace_ok(t0));
    trace_write(t1, 0, 0);
    BOOST_TEST(trace_ok(t1));
    trace_write(t1, 1, 0);
    BOOST_TEST(!trace_ok(t1));
    trace_write(t2, 3, 4);
    BOOST_TEST(!trace_ok(t2));

    trace_print(&traces);

    trace_deinit(&traces);
}
