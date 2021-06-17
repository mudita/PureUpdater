#include <errno.h>

_ssize_t _write(int fd, const char *buf, size_t nbyte);

_ssize_t
_write(int fd __attribute__((unused)), const char *buf __attribute__((unused)),
       size_t nbyte __attribute__((unused)))
{
    // STDOUT and STDERR are routed to the trace device
    if (fd == 1 || fd == 2)
    {
    }
    else
    {
      errno = ENOSYS;
      return -1;
    }
}

// ----------------------------------------------------------------------------

//#endif // !defined(OS_USE_SEMIHOSTING) && !(__STDC_HOSTED__ == 0)
