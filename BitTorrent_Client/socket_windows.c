/*
 * Author: Daniel Liscinsky
 */

#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include "socket_windows.h"
#undef socket



int posix_socket(int domain, int type, int protocol) {


	
	socket_t sock = socket(domain, type, IPPROTO_UDP);
	INVALID_SOCKET;

	errno = 1;
	
}