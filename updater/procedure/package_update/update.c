#include <common/enum_s.h>
#include "update.h"

const char *update_strerror(int e)
{
    switch(e)
    {
        ENUMS(ErrorUpdateOk);
        ENUMS(ErrorUnpack);
        ENUMS(ErrorFS);
    }
    return "UNKNOWN";
}
