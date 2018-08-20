/*
 * Author: Daniel Liscinsky
 */

#if defined(_WIN32)

#endif

#include "utp.h"

#include <unordered_map>
#include <mutex>
#include <atomic>


using namespace std;



// 
// Internal data structures for storing the state of a uTP socket / connection
// 

/**
 * 
 */
struct utp_socket_state {
	socket_t sock_fd;
	std::mutex lock; // 

	uint8_t state;
	uint16_t recv_conn_id;
	uint16_t send_conn_id;
	;//timestamp stuffffffffffffffff
	uint32_t wnd_size;
	uint16_t seq_nr;
	uint16_t ack_nr;
};



#define UTP_HDR_TYPE(type_ver_field) (type_ver_field >> 4)

/*
0       4       8               16              24              32
+-------+-------+---------------+---------------+---------------+
| type  | ver   | extension     | connection_id                 |
+-------+-------+---------------+---------------+---------------+
| timestamp_microseconds                                        |
+---------------+---------------+---------------+---------------+
| timestamp_difference_microseconds                             |
+---------------+---------------+---------------+---------------+
| wnd_size                                                      |
+---------------+---------------+---------------+---------------+
| seq_nr                        | ack_nr                        |
+---------------+---------------+---------------+---------------+
*/
PACKED(
struct utp_hdr {
	uint8_t type_ver;
	uint8_t extension;
	uint16_t connection_id;
	uint32_t timestamp_microseconds;
	uint32_t timestamp_difference_microseconds;
	uint32_t wnd_size;
	uint16_t seq_nr;
	uint16_t ack_nr;
});



/**
 * 
 */
static unordered_map<socket_t, struct utp_socket_state *> socket_utp_states;
static mutex mutex_state_map;



#if defined(_WIN32)
void wsa_set_errno(int wsa_err) {					//TODO is this really necessary or even useful? Should I just remove this function and all references to it...
	switch (wsa_err) {
	case WSANOTINITIALISED:
		//errno = ;
		break;
	case WSAENETDOWN:
		//errno = ;
		break;
	case WSAEAFNOSUPPORT:
		errno = EAFNOSUPPORT;
		break;
	case WSAEINPROGRESS:
		//errno = ;
		break;
	case WSAEMFILE:
		errno = EMFILE;
		break;
	case WSAEINVAL:
		errno = EINVAL;
		break;
	case WSAEINVALIDPROVIDER:
		//errno = ;
		break;
	case WSAEINVALIDPROCTABLE:
		//errno = ;
		break;
	case WSAENOBUFS:
		errno = ENOBUFS;
		break;
	case WSAEPROTONOSUPPORT:
		errno = EPROTONOSUPPORT;
		break;
	case WSAEPROTOTYPE:
		//errno = ;
		break;
	case WSAEPROVIDERFAILEDINIT:
		//errno = ;
		break;
	case WSAESOCKTNOSUPPORT:
		//errno = ;
		break;
	default:
		//errno = ;
	}
}
#endif

/**
 * Looks up the internal state of the uTP socket.
 * 
 * @return The state of the uTP socket, or NULL if the socket handle is invalid.
 */
struct utp_socket_state * utp_lookup_socket_state(socket_t sock_fd) {

	struct utp_socket_state *sock_state = NULL;

	// Lookup socket state
	mutex_state_map.lock();
	auto key_val = socket_utp_states.find(sock_fd);

	// If sock_fd is valid
	if (key_val != socket_utp_states.end()) {
		sock_state = key_val->second; // Save off pointer to socket state
	}
	mutex_state_map.unlock();

	return sock_state;
}



socket_t utp_socket(int domain) {
	
	// Create the underlying socket
	socket_t sock_fd = socket(domain, SOCK_DGRAM, IPPROTO_UDP);
	if (sock_fd == -1) {
#if defined(_WIN32)
		wsa_set_errno(WSAGetLastError()); // Set errno
#endif
		return sock_fd;
	}

	// Create initial socket state
	struct utp_socket_state *sock_state = (struct utp_socket_state *) malloc(sizeof(struct utp_socket_state));
	if (!sock_state) {
		// errno has already been set
#if defined(_WIN32)
		WSASetLastError(WSA_NOT_ENOUGH_MEMORY);
		return INVALID_SOCKET;
#else
		retrun -1;
#endif
	}

	sock_state->sock_fd = sock_fd;
	sock_state->;//TODO
	sock_state->;
	sock_state->;
	sock_state->;//TODO

	// Add socket state object to global internal state
	mutex_state_map.lock();
	socket_utp_states[sock_fd] = sock_state;
	mutex_state_map.unlock();

	return sock_fd;
}

int utp_connect(socket_t sock_fd, const struct sockaddr *addr, socklen_t addrlen) {

	struct utp_socket_state *sock_state = utp_lookup_socket_state(sock_fd);
	

	// If sock_fd was NOT valid (lookup of internal state failed)
	if (!sock_state) {

		//TODO err
	}


	// Ensure only one thread can modify this socket's internal state at a time
	sock_state->lock.lock();
	__try {

		// 

		//TODO



	}
	__finally {
		// Always release the socket's mutex before returning
		sock_state->lock.unlock();
	}

	return ??? ;
}

int utp_listen(int sock_fd, int backlog) {

	struct utp_socket_state *sock_state = utp_lookup_socket_state(sock_fd);
	

	// If sock_fd was NOT valid (lookup of internal state failed)
	if (!sock_state) {

		//TODO err
	}


	// Ensure only one thread can modify this socket's internal state at a time
	sock_state->lock.lock();
	__try {

		// 

		//TODO



	}
	__finally {
		// Always release the socket's mutex before returning
		sock_state->lock.unlock();
	}

	return ??? ;
}

int utp_accept(socket_t sock_fd, struct sockaddr *addr, socklen_t *addrlen) {

	struct utp_socket_state *sock_state = utp_lookup_socket_state(sock_fd);


	// If sock_fd was NOT valid (lookup of internal state failed)
	if (!sock_state) {

		//TODO err
	}


	// Ensure only one thread can modify this socket's internal state at a time
	sock_state->lock.lock();
	__try {

		// 

		//TODO



	}
	__finally {
		// Always release the socket's mutex before returning
		sock_state->lock.unlock();
	}

	return ??? ;
}

ssize_t utp_recv(socket_t sock_fd, void *buf, size_t len, int flags) {

	struct utp_socket_state *sock_state = utp_lookup_socket_state(sock_fd);


	// If sock_fd was NOT valid (lookup of internal state failed)
	if (!sock_state) {

		//TODO err
	}


	// Ensure only one thread can modify this socket's internal state at a time
	sock_state->lock.lock();
	__try {

		// 

		//TODO



	}
	__finally {
		// Always release the socket's mutex before returning
		sock_state->lock.unlock();
	}

	return ??? ;
}

ssize_t utp_recvfrom(socket_t sock_fd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen) {

	struct utp_socket_state *sock_state = utp_lookup_socket_state(sock_fd);


	// If sock_fd was NOT valid (lookup of internal state failed)
	if (!sock_state) {

		//TODO err
	}


	// Ensure only one thread can modify this socket's internal state at a time
	sock_state->lock.lock();
	__try {

		// 

		//TODO



	}
	__finally {
		// Always release the socket's mutex before returning
		sock_state->lock.unlock();
	}

	return ??? ;
}

