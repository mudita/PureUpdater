#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <common/log.h>
#include <common/json.h>
#include <common/types.h>


bool checksum_verify_all(verify_file_handle_s *handle, const char *tmp_path);

bool checksum_verify(verify_file_handle_s *handle);

#ifdef __cplusplus
}
#endif
