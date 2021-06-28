#pragma once

//! Volume status structure
struct statvfs {
	unsigned long f_bsize;      //! Fileystem block size
	unsigned long f_frsize;     //! Fragment size
	unsigned long f_blocks;     //! Size of fs in f_frsize units
	unsigned long f_bfree;      //! Number of free blocks
};

/**
 * @brief Returns information about a mounted filesystem.
 * @param path pathname of any file within the mounted filesystem 
 * @param buf pointer to a statvfs structure @see statvfs
 * @return  On success, zero is returned.  On error, -1 is returned, and errno is set appropriately.
 */
int statvfs( const char* path, struct statvfs* buf);