#include <common/enum_s.h>
#include "common/trace.h"
#include "update.h"
#include "string.h"
#include "priv_update.h"

const char *update_strerror(int e)
{
    switch(e)
    {
        ENUMS(ErrorUpdateOk);
        ENUMS(ErrorUnpack);
        ENUMS(ErrorFS);
        ENUMS(ErrorUpdateTODO);
    }
    return "UNKNOWN";
}


bool update_firmware(struct update_handle_s *handle, trace_list_t *tl)
{
    (void) handle;
    trace_t *t = trace_append("update",tl, update_strerror, NULL);

    if (!unpack(handle,tl))
    {
        trace_write(t, ErrorUnpack, 0);
    }
    trace_write(t, ErrorUpdateTODO, 0);
    return trace_ok(t);
}
