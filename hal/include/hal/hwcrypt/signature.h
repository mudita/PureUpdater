#pragma once

//! Error codes for verificate signatur3
enum sec_verify_error
{
    sec_verify_ok,         //! Verification OK
    sec_verify_openconfig, //! Open configuration
    sec_verify_invalsign,  //! Unsigned binary
    sec_verify_ioerror,    //! Check errno
    sec_verify_invalevt,   //! Invalid evt vector
    sec_verify_confopen    //! Configuration is open
};

/** Check executable binary signature
 * @param[in] path_bin Path to the executable binary
 * @return Verificaiton error see @sec_verify_error
 */
int sec_verify_executable(const char *path_bin);

/** Check resource binary signature
 * @param[in] path_bin Path to the executable binary
 * @param[in] signature_file Path to the signature
 * @return Verificaiton error see @sec_verify_error
 */
int sec_verify_file(const char *file, const char *signature_file);