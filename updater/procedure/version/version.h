#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <common/log.h>
#include <common/version_json.h>
#include <common/types.h>

bool version_check_all(verify_file_handle_s *handle, const char *tmp_path, bool allow_downgrade);

bool version_check(verify_file_handle_s *handle, bool allow_downgrade);


#ifdef __cplusplus
}
#endif
