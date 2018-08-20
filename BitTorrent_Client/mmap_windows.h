/*
 * mmap() replacement for Windows
 *
 * Author: Daniel Liscinsky
 */

#ifndef WINDOWS_MMAP_H
#define WINDOWS_MMAP_H


#include <memoryapi.h>
#include <sys/types.h>


#if defined(__cplusplus)
extern "C" {
#endif


#define PROT_READ     0x1
#define PROT_WRITE    0x2
#ifdef FILE_MAP_EXECUTE /* This flag is only available in WinXP+ */
#define PROT_EXEC     0x4
#else
#define PROT_EXEC        0x0
#define FILE_MAP_EXECUTE 0
#endif

#define MAP_SHARED    0x01
#define MAP_PRIVATE   0x02
#define MAP_ANONYMOUS 0x20
#define MAP_ANON      MAP_ANONYMOUS

#define MAP_FAILED    ((void *) -1)


#ifdef __USE_FILE_OFFSET64
#define __USE_FILE_OFFSET64_
#endif



void * mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset);
int munmap(void *addr, size_t length);



#if defined(__cplusplus)
}
#endif

#endif