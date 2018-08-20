/*
 * Author: Daniel Liscinsky
 */

#pragma once

#ifndef _UTORRENT_TRANSPORT_PROTOCOL_H
#define _UTORRENT_TRANSPORT_PROTOCOL_H


#if defined(_WIN32)
#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#else
#include <sys/types.h>
#include <sys/socket.h>
 //#include <arpa/inet.h>

	#define closesocket(x) close(x)
#endif

//#include <stdint.h>

#include "inet_cross_platform.h"



/*
See the man page for socket(2) for details on socket creation. The type 
and protocol of a uTP socket are always SOCK_DGRAM and IPPROTO_UDP 
(hence those parameters are omitted for this function).


 */
socket_t utp_socket(int domain);

int utp_connect(socket_t sockfd, const struct sockaddr *addr, socklen_t addrlen);
int utp_listen(int sockfd, int backlog);
int utp_accept(socket_t sockfd, struct sockaddr *addr, socklen_t *addrlen);

ssize_t utp_recv(socket_t sockfd, void *buf, size_t len, int flags);
ssize_t utp_recvfrom(socket_t sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);

#endif