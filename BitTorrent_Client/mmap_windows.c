/*
 * Changes:
 *	* Updated to include a header file.
 *	* Corrected return type of munmap().
 *	* Fixed leaked handle from CreateFileMapping().
 *	* TODO			Sets errno on failure.
 *
 * Author: Daniel Liscinsky
 */

/* mmap() replacement for Windows
 *
 * Author: Mike Frysinger <vapier@gentoo.org>
 * Placed into the public domain
 */

/*
 * m-labs/uclibc-lm32 is licensed under the GNU Lesser General Public License v2.1.
 * https://github.com/m-labs/uclibc-lm32/blob/master/utils/mmap-windows.c
 */

/* References:
 * CreateFileMapping: http://msdn.microsoft.com/en-us/library/aa366537(VS.85).aspx
 * CloseHandle:       http://msdn.microsoft.com/en-us/library/ms724211(VS.85).aspx
 * MapViewOfFile:     http://msdn.microsoft.com/en-us/library/aa366761(VS.85).aspx
 * UnmapViewOfFile:   http://msdn.microsoft.com/en-us/library/aa366882(VS.85).aspx
 */

#include <io.h>
#include <windows.h>

#include "mmap_windows.h"



#ifdef __USE_FILE_OFFSET64_
# define DWORD_HI(x) (x >> 32)
# define DWORD_LO(x) ((x) & 0xffffffff)
#else
# define DWORD_HI(x) (0)
# define DWORD_LO(x) (x)
#endif

void * mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset)
{
	if (prot & ~(PROT_READ | PROT_WRITE | PROT_EXEC)) {
	
		//TODO errno
		
		return MAP_FAILED;
	}
	if (fd == -1) {
		if (!(flags & MAP_ANON) || offset) {
			
			//TODO errno

			return MAP_FAILED;
		}
	} else if (flags & MAP_ANON){
	
		//TODO errno
		
		return MAP_FAILED;
	}

	DWORD flProtect;
	if (prot & PROT_WRITE) {
		if (prot & PROT_EXEC)
			flProtect = PAGE_EXECUTE_READWRITE;
		else
			flProtect = PAGE_READWRITE;
	} else if (prot & PROT_EXEC) {
		if (prot & PROT_READ)
			flProtect = PAGE_EXECUTE_READ;
		else if (prot & PROT_EXEC)
			flProtect = PAGE_EXECUTE;
	} else
		flProtect = PAGE_READONLY;

	off_t end = length + offset;
	HANDLE mmap_fd, h;
	if (fd == -1)
		mmap_fd = INVALID_HANDLE_VALUE;
	else
		mmap_fd = (HANDLE)_get_osfhandle(fd);
	h = CreateFileMapping(mmap_fd, NULL, flProtect, DWORD_HI(end), DWORD_LO(end), NULL);
	if (h == NULL) {
		
		//TODO errno
		//GetLastError()

		return MAP_FAILED;
	}

	DWORD dwDesiredAccess;
	if (prot & PROT_WRITE)
		dwDesiredAccess = FILE_MAP_WRITE;
	else
		dwDesiredAccess = FILE_MAP_READ;
	if (prot & PROT_EXEC)
		dwDesiredAccess |= FILE_MAP_EXECUTE;
	if (flags & MAP_PRIVATE)
		dwDesiredAccess |= FILE_MAP_COPY;
	void *ret = MapViewOfFile(h, dwDesiredAccess, DWORD_HI(offset), DWORD_LO(offset), length);
	if (ret == NULL) {
		ret = MAP_FAILED;
		
		//TODO errno
		//GetLastError()
	}

	// We can close this handle now
	CloseHandle(h);
	
	return ret;
}

int munmap(void *addr, size_t length)
{
	if (UnmapViewOfFile(addr)) {
		return 0;
	}

	//TODO errno

	return -1;
}

#undef DWORD_HI
#undef DWORD_LO
