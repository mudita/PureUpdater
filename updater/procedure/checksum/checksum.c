#include <common/enum_s.h>
#include <common/trace.h>
#include "checksum.h"


bool checksum_verify(struct backup_handle_s *handle, trace_list_t *tl)
{
    (void)handle;
    trace_append("checksum", tl, strerror_checksum, strerror_checksum_ext);

    return false;
}

const char *strerror_checksum(int err)
{
    switch(err) {
        ENUMS(ErrorChecksumOk);
    }
    return "UNKNOWN";
}

const char *strerror_checksum_ext(int err, int err_ext)
{
    (void)err; (void)err_ext;
    return "UNKNOWN";
}
