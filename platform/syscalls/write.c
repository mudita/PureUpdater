#include <errno.h>
#include <console/console.h>

_ssize_t _write(int fd, const char *buf, size_t nbyte) 
{
    // STDOUT and STDERR are routed to the trace device
    if (fd == 1 || fd == 2)
    { 
      int error = debug_console_write(buf, nbyte);
      if(error<0) {
        errno = -error;
        return -1;
      } else {
        return error;
      }
    }
    else
    {
      errno = ENOSYS;
      return -1;
    }
}