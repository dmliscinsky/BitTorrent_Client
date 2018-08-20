/*
 * Author: Daniel Liscinsky
 */

#pragma once

#ifndef WINDOWS_SOCKET_H
#define WINDOWS_SOCKET_H


#define socket(domain, type, protocol) posix_sock(domain, type, protocol)


#define close(x) closesocket(x)



typedef SOCKET socket_t;
typedef SSIZE_T ssize_t;



//#include <stdint.h>



/*
See the man page for socket(2) for details on socket creation.
 */
int posix_socket(int domain, int type, int protocol);



#endif