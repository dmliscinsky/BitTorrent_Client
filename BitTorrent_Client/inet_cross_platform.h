/*
 * 
 * 
 * The socket_t type exists because of the size and signedness 
 * difference of socket handles on Windows vs Linux.
 * 
 * 
 * Author: Daniel Liscinsky
 */

#ifndef _INET_CROSS_PLATFORM_H
#define _INET_CROSS_PLATFORM_H



/*
 * Wrap a struct definition with this macro to pack its members.
 */
#if defined(_MSC_VER) // If using Microsoft's Visual C++ compiler
#define PACKED(__Declaration__) \
	__pragma(pack(push, 1)) \
	__Declaration__ \
	__pragma(pack(pop))
#elif defined(__GNUC__) // If being compiled by GCC, or a non-GCC compiler that claims to accept the GNU C dialects
#define PACKED(__Declaration__) __Declaration__ __attribute__((__packed__)) 
#endif



#if defined(_WIN32)
#include <ws2tcpip.h>

typedef SOCKET socket_t;
typedef SSIZE_T ssize_t;

#else
#include <endian.h>

typedef int socket_t;

#define ntohll(x) be64toh(x)
#define	htonll(x) htobe64(x)
#endif


#endif