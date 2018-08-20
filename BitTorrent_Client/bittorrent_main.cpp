/*
 * Author: Daniel Liscinsky
 */

//#define _LARGEFILE64_SOURCE

#if defined(_WIN32)

#define __USE_FILE_OFFSET64
#include "mmap_windows.h"

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <io.h>

#pragma comment(lib, "ws2_32.lib")

	#define open(filepath, flags, ...) _open(filepath, flags, __VA_ARGS__)
	#define close(fd)	_close(fd)

#else

#define _FILE_OFFSET_BITS 64
#define O_BINARY // Dummy flag for open(), needed on Windows but does not exist on *nix

#include <unistd.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <arpa/inet.h>
	
	#define closesocket(x) close(x)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>

#include <thread>
#include <atomic>
#include <string>

#include "inet_cross_platform.h"
#include "bencode.h"
#include "bittorrent.h"
#include "sha1.h"


using namespace std;
using namespace bencode;
using namespace bittorrent;



#define BUFFER_RESIZE_FACTOR 2



int read_bencoded_dict_file(const char *filepath, bencoded_dict *dict) {

	FILE *file_in;
	int result = 0;

#if defined(_WIN32)
	if (fopen_s(&file_in, filepath, "rb")) {
		return result;
	}
#else
	file_in = fopen(filepath, "rb");
	if (!file_in) {
		return result;
	}
#endif
	
	size_t buf_size = 1024; // The size of the allocated buffer
	size_t buf_len = 0; // The current length of the data in the buffer in bytes
	char * buf = (char *) malloc(buf_size);
	if (!buf) {
		fclose(file_in);
		return result;
	}
	
	size_t items_read;
	while ( (items_read = fread(buf + buf_len, 1, buf_size - buf_len, file_in)) ) {
		
		buf_len += items_read;

		// Resize buffer if out of space
		if (buf_len == buf_size) {
			
			size_t new_buf_size = buf_size * BUFFER_RESIZE_FACTOR;
			char *new_buf = (char *) realloc(buf, new_buf_size);
			if (!new_buf) {
				free(buf);
				fclose(file_in);
				return result;
			}
			
			buf = new_buf;
			buf_size = new_buf_size;
		}
	}
	
	
	// Parse the bencoded file data
	size_t len_remaining = 0;
	char *buf_remaining = bencode_parse_dict(buf, buf_len, &len_remaining, dict);
	if (!buf_remaining) {
		free(buf);
		fclose(file_in);
		return result;
	}

	result = 1;

fail:
	fclose(file_in);
	return result;
}



//
bittorrent::torrent_file torrent;

uint8_t my_peer_id[BITTORRENT_PEER_ID_LEN + 1] = { 0 }; // My peer id (guaranteed to be null termianted)

//#define _LEN
bool should_get_compact_peer_list = 1;

uint16_t my_listen_port = 6881;

atomic_uint next_transaction_id;

uint64_t bytes_left = 5000; // Number of bytes left to download
uint64_t bytes_downloaded = 0; // Total number of bytes downloaded so far
uint64_t bytes_uploaded = 0; // Total number of bytes uploaded so far


//atomic< vector<bittorrent_peer> > peers;




/*
 *
 */
uint8_t * generate_peer_id() {

	//TODO make random
	memcpy(my_peer_id, "12345678901234567890", BITTORRENT_PEER_ID_LEN);

	return my_peer_id;
}

/*
 *
 */
char * ultostr(unsigned long i, int radix) {
	/*s
	char *number_str = malloc();
	if (number_str) {

	}

	return number_str;
	*/

	//atomic< vector<socket_t> > peers;
	//peers.operator std::vector<socket_t, std::allocator<socket_t>>();


	char a[1] = "";
	return a;
}

void print_bencoded_dict_keys() {
	/*
	for (auto e : *this) {
		printf("%d:", e.first.length);
		for (int i = 0; i < e.first.length; i++)
			printf("%c", e.first.byte_str[i]);
		printf("\n");
	}
	*/
}

int update_tracker_addrinfo(struct tracker *tracker) {

	printf("announcer host = %s\n", tracker->host.c_str());//TODO
	printf("announcer port = %s\n", tracker->port);
	printf("announcer uri  = %s\n", tracker->uri.c_str());


	
	struct addrinfo hints = { 0 };
	hints.ai_socktype = tracker->socktype;

	// Get address info for tracker
	if (getaddrinfo(tracker->host.c_str(), tracker->port, &hints, &tracker->addr)) {

		//TODO
		printf("getaddrinfo() failed\n");
		//printf("%s\n", gai_strerror(getaddrinfo(announcer.host.c_str(), announcer.port, &hints, &announcer.addr)));
		return 0;
	}


	//TODO
	printf("AF_INET = %x\n", AF_INET);//TODO
	printf("SOCK_STREAM = %x\n", SOCK_STREAM);
	printf("SOCK_DGRAM  = %x\n\n", SOCK_DGRAM);

	//TODO 
	struct addrinfo *ai_curr = tracker->addr;
	do {

		printf("ai_family = %x\n", ai_curr->ai_family);
		printf("addr->sa_family = %x\n", ai_curr->ai_addr->sa_family);
		if (ai_curr->ai_addr->sa_family == AF_INET) {
			printf("ipv4 = %x\n", ((struct sockaddr_in *)ai_curr->ai_addr)->sin_addr);
			char buf[16];
			inet_ntop(ai_curr->ai_addr->sa_family, &((struct sockaddr_in *)ai_curr->ai_addr)->sin_addr, buf, sizeof(buf));
			printf("ipv4 = %s\n", buf);
		}

		printf("ai_socktype = %x\n", ai_curr->ai_socktype);
		printf("ai_protocol = %x\n", ai_curr->ai_protocol);

		printf("ai_next = %p\n\n", ai_curr->ai_next);

		ai_curr = ai_curr->ai_next;
	} while (ai_curr);

	return 1;
}

/**
 * 
 */
int bittorrent_talk_with_peer() {
	
	ssize_t bytes_sent, bytes_recvd;


	//TODO
	struct sockaddr_in __addr = { 0 };
	
	struct addrinfo _addr, *addr;
	addr = &_addr;
	addr->ai_family = AF_INET;
	addr->ai_socktype = SOCK_DGRAM;//SOCK_STREAM;
	addr->ai_protocol = 0;
	addr->ai_flags = 0;
	addr->ai_addr = (struct sockaddr *)&__addr;
	addr->ai_addrlen = sizeof(struct sockaddr_in);

	inet_pton(AF_INET, "209.194.253.131", &((struct sockaddr_in *)addr->ai_addr)->sin_addr);
	((struct sockaddr_in *)addr->ai_addr)->sin_port = htons(62701); //TODO port
	((struct sockaddr_in *)addr->ai_addr)->sin_family = AF_INET;
	memset( ((struct sockaddr_in *)addr->ai_addr)->sin_zero, 0, 8);




	socket_t sock_peer = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
#if defined(_WIN32)
	if (sock_peer == INVALID_SOCKET) {
#else
	if (sock_tracker < 0) {
#endif
		printf("failed to open scoket\n");//TODO
		return 0;
	}

	if (connect(sock_peer, addr->ai_addr, addr->ai_addrlen) < 0) {

		printf("failed to connect to peer\n");//TODO
		closesocket(sock_peer);
		return 0;
	}

	
	// Build my handshake message
	struct bittorrent_handshake handshake;
	//handshake.proto_name_len = 19;
	memcpy(handshake.proto_name, "Bittorrent protocol", handshake.proto_name_len);
	memcpy(handshake.info_hash, torrent.info_hash, SHA1_HASH_LENGTH);
	memcpy(handshake.peer_id, my_peer_id, BITTORRENT_PEER_ID_LEN);

	// Send handshake message
	bytes_sent = send(sock_peer, (char *)&handshake, sizeof(struct bittorrent_handshake), 0);
	if (bytes_sent < 0) {

		//TODO err
		printf("send() failed\n");

		//TODO cleanup
		closesocket(sock_peer);
		return 0;
	}


	// Receive handshake message
	bytes_recvd = recv(sock_peer, (char *)&handshake, sizeof(struct bittorrent_handshake), 0);
	if (bytes_recvd < 0) {
		
		//TODO err
		printf("recv() failed\n");

		//TODO cleanup
		closesocket(sock_peer);
		return 0;
	}
	/*
	// Check response is for our request
	if (ntohl(conn_resp.action) != UDP_TRACKER_ACTION_CONNECT || ntohl(conn_resp.transaction_id) != transaction_id) {

		//TODO idk ????????
	}

	// Store connection ID
	connection_id = ntohll(conn_resp.connection_id);
	printf("connection_id = %llu\n", connection_id);//TODO
	*/

	printf("sutff: ");
	for (unsigned int i = 0; i < bytes_recvd; i++) {
		printf("%02x ", ((char *)&handshake)[i]);
	}
	printf("\n");




	printf("proto_name_len = %u\n", handshake.proto_name_len);
	for (unsigned int i = 0; i < handshake.proto_name_len; i++) {
		printf("%c", handshake.proto_name[i]);
	}
	printf("\n");
	printf("info_hash: ");
	for (unsigned int i = 0; i < 20; i++) {
		printf("%02x ", handshake.info_hash[i]);
	}
	printf("\n");

	printf("MY info_hash: ");
	for (unsigned int i = 0; i < 20; i++) {
		printf("%02x ", torrent.info_hash[i]);
	}
	printf("\n");

	printf("peer_id: ");
	for (unsigned int i = 0; i < 20; i++) {
		printf("%02x ", handshake.peer_id[i]);
	}
	printf("\n");

	


	closesocket(sock_peer);
	return 1;
}

/**
 * 
 */
int send_udp_tracker_message() {


	return 0;
}

/**
 * 
 */
int recv_udp_tracker_message() {


	return 0;
}

/**
 * 
 */
int connect_to_tracker_udp() {

	struct tracker &tracker = torrent.announcer_list[3];//TODO

	ssize_t bytes_sent, bytes_recvd;

	uint32_t transaction_id;
	uint64_t connection_id;


	// Get address info for tracker
	if (!update_tracker_addrinfo(&tracker)) {
		return 0;
	}


	socket_t sock_tracker = socket(tracker.addr->ai_family, tracker.addr->ai_socktype, tracker.addr->ai_protocol);
#if defined(_WIN32)
	if (sock_tracker == INVALID_SOCKET) {
#else
	if (sock_tracker < 0) {
#endif
		printf("failed to open scoket\n");//TODO
		return 0;
	}

	if (connect(sock_tracker, tracker.addr->ai_addr, tracker.addr->ai_addrlen) < 0) {

		printf("failed to connect to tracker\n");//TODO
		closesocket(sock_tracker);
		return 0;
	}


start_connection:
	// Build connect request message
	transaction_id = next_transaction_id++;
	struct tracker_connect_req_msg conn_req = {
		htonll(UDP_TRACKER_PROTOCOL_ID),
		htonl(UDP_TRACKER_ACTION_CONNECT),
		htonl(transaction_id)
	};

	// Send connect request message
	bytes_sent = send(sock_tracker, (char *)&conn_req, sizeof(struct tracker_connect_req_msg), 0);
	if (bytes_sent < 0) {

		//TODO err
		printf("send() failed\n");

		//TODO cleanup
		closesocket(sock_tracker);
		return 0;
	}


	// Receive connect response message
	struct tracker_connect_resp_msg conn_resp = { 0 };
	bytes_recvd = recv(sock_tracker, (char *)&conn_resp, sizeof(struct tracker_connect_resp_msg), 0);
	if (bytes_recvd < 0) {

		//TODO handle timeouts........

		//TODO err
		printf("recv() failed\n");

		//TODO cleanup
		closesocket(sock_tracker);
		return 0;
	}
	
	// Check response is for our request
	if (ntohl(conn_resp.action) != UDP_TRACKER_ACTION_CONNECT || ntohl(conn_resp.transaction_id) != transaction_id) {
		
		//TODO idk ????????
	}

	// Store connection ID
	connection_id = ntohll(conn_resp.connection_id);
	printf("connection_id = %llu\n", connection_id);//TODO


	do {
		// Build announce request message
		transaction_id = next_transaction_id++;
		struct tracker_announce_req_msg announce_req = {
			htonll(connection_id),
			htonl(UDP_TRACKER_ACTION_ANNOUNCE),
			htonl(transaction_id),
			{0},
			{0},
			htonll(bytes_downloaded),
			htonll(bytes_left),
			htonll(bytes_uploaded),
			0, //event			//TODO
			0, 
			htonl(-1),
			htons(my_listen_port)
		};
		memcpy(announce_req.info_hash, torrent.info_hash, SHA1_HASH_LENGTH);
		memcpy(announce_req.peer_id, my_peer_id, BITTORRENT_PEER_ID_LEN);

		// Send announce request message
		bytes_sent = send(sock_tracker, (char *)&announce_req, sizeof(struct tracker_announce_req_msg), 0);
		if (bytes_sent < 0) {

			//TODO handle timeouts........

			//TODO err
			printf("send() failed\n");

			//TODO cleanup
			closesocket(sock_tracker);
			return 0;
		}


		// Receive announce response message
		uint8_t buf[8192];
		bytes_recvd = recv(sock_tracker, (char *)&buf, sizeof(buf), 0);//TODO do I need multiple recvs ?????
		if (bytes_recvd < 0) {

			//TODO handle timeouts........

			//TODO err
			printf("recv() failed\n");

			//TODO cleanup
			closesocket(sock_tracker);
			return 0;
		}

		struct tracker_announce_resp_msg *announce_resp = (struct tracker_announce_resp_msg *) buf;

		// Check response is for our request
		if (ntohl(announce_resp->action) != UDP_TRACKER_ACTION_ANNOUNCE || ntohl(announce_resp->transaction_id) != transaction_id) {

			//TODO idk ????????
		}


		// Process announce response
		printf("bytes_recvd = %d\n", bytes_recvd);//TODO
		printf("interval = %u\n", ntohl(announce_resp->interval));//TODO
		printf("leechers = %u\n", ntohl(announce_resp->leechers));
		printf("seeders = %u\n", ntohl(announce_resp->seeders));

		if (tracker.addr->ai_family == AF_INET) {
			
			unsigned int ip_bytes = 4; // Size of the IP address in bytes
			const unsigned int port_bytes = 2; // Size of port number in bytes

			for (int i = 0; i < ntohl(announce_resp->seeders) + ntohl(announce_resp->leechers); i++) {

				struct sockaddr_in addr = { 0 };
				addr.sin_family = AF_INET;
				memcpy(&addr.sin_addr, &announce_resp->ip_port_list[i * (ip_bytes + port_bytes)], ip_bytes);
				memcpy(&addr.sin_port, &announce_resp->ip_port_list[i * (ip_bytes + port_bytes) + ip_bytes], port_bytes);

				char __buf[16];
				inet_ntop(AF_INET, &addr.sin_addr, __buf, sizeof(__buf));
				printf("ipv4 = %s:%s\n", __buf, std::to_string(addr.sin_port).c_str());
			}
		}


	} while (0); //TODO make multiple announce requests at regular intervals to check for new peers


	closesocket(sock_tracker);
	return 1;
}

/**
 * 
 */
int connect_to_tracker_tcp() {
	
	struct tracker &tracker = torrent.announcer_list[1];//TODO


	// Get address info for tracker
	if (!update_tracker_addrinfo(&tracker)) {
		return 0;
	}

	
	socket_t sock_tracker = socket(tracker.addr->ai_family, tracker.addr->ai_socktype, tracker.addr->ai_protocol);
#if defined(_WIN32)
	if (sock_tracker == INVALID_SOCKET) {
#else
	if (sock_tracker < 0) {
#endif
		printf("failed to open scoket\n");//TODO
		return 0;
	}
	
	if (connect(sock_tracker, tracker.addr->ai_addr, tracker.addr->ai_addrlen) < 0) {
		
		printf("failed to connect to tracker\n");//TODO
		closesocket(sock_tracker);
		return 0;
	}
	

	//char query_tracker_msg[] = "GET /announce?peer_id=aaaaaaaaaaaaaaaaaaaa&info_hash=aaaaaaaaaaaaaaaaaaaa&port=6881&left=0&downloaded=0&uploaded=0&compact=0";

	// Build request to send to tracker
	string query_tracker_msg("GET ");
	query_tracker_msg += tracker.uri;
	query_tracker_msg += "?info_hash=" + torrent.info_hash_str;
	query_tracker_msg += "&peer_id=" + string((char *)my_peer_id);//TODO need to percent encode..........
	query_tracker_msg += "&port=" + std::to_string(my_listen_port);
	query_tracker_msg += "&left=" + std::to_string(bytes_left);
	query_tracker_msg += "&downloaded=" + std::to_string(bytes_downloaded);
	query_tracker_msg += "&uploaded=" + std::to_string(bytes_uploaded);
	query_tracker_msg += "&compact=" + string(should_get_compact_peer_list ? "1" : "0");
	//query_tracker_msg += "&ip=127.0.0.1";
	query_tracker_msg += " HTTP/1.1\r\n";
	query_tracker_msg += "Host: " + tracker.host + ":" + string(tracker.port) + "\r\n";
	query_tracker_msg += "Accept: application/x-bittorrent\r\n";
	query_tracker_msg += "Connection: Keep-Alive\r\n";
	query_tracker_msg += "\r\n";
	

	// 
	ssize_t bytes_sent = send(sock_tracker, query_tracker_msg.c_str(), query_tracker_msg.length(), 0);
	if (bytes_sent < 0) {

		//TODO err
		printf("send() failed\n");

		//TODO cleanup
		closesocket(sock_tracker);
		return 0;
	}

	char buf[8192];
	ssize_t bytes_recvd = recv(sock_tracker, buf, sizeof(buf), 0);
	if (bytes_recvd < 0) {

		//TODO err
		printf("recv() failed\n");

		//TODO cleanup
		closesocket(sock_tracker);
		return 0;
	}

	printf("recv = %d\n", bytes_recvd);

	for (int i = 0; i < bytes_recvd; i++) {
		printf("%02x ", buf[i]);
	}
	printf("\n");

	
	closesocket(sock_tracker);
	return 1;
}

/**
 * 
 * 
 * @return Zero on failure, and non-zero on success.
 */
int download_torrent(bittorrent::torrent_file *torrent) {

	// Start threads to connect to trackers












	/*
	int file_fd = open("test_file.txt", O_RDWR | O_CREAT, // | O_BINARY
	//| O_LARGEFILE
	S_IREAD | S_IWRITE
	//S_IRUSR | S_IWUSR
	);
	if (file_fd < 0) {

	//TODO
	printf("failed to open file\n");
	printf("errno = %d\n", errno);
	return 0;
	}

	char *mapped_file = (char *) mmap(NULL, 0x4000, PROT_WRITE, MAP_SHARED, file_fd, 0);
	if (mapped_file == MAP_FAILED) {

	//TODO
	printf("failed to map file\n");
	printf("errno = %d\n", errno);

	close(file_fd);
	return 0;
	}

	printf("mapped_file[9] = %c\n", mapped_file[9]);
	printf("mapped_file[0x3ffe] = 0x%02x\n", mapped_file[0x3ffe]);
	mapped_file[0] = '0';
	printf("hi 2\n");
	mapped_file[0x3fff] = 'e';
	printf("hi 3\n");
	munmap(mapped_file, 0x4000);

	*/

	return 1;
}



int main(int argc, char **argv) {

	int err = 0;
	char *torrent_filepath = NULL;


#if defined(_WIN32)
	WSADATA wsaData = { 0 };

	err = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if(err) {
		err; //TODO
	}
#endif
	

	// Parse command line


	/*
	char tmp[] = "d5:hello3:ABCe";
	unsigned int length = strlen(tmp);
	unsigned int remaining = length;

	bencoded_value *value;
	char *end = bencode_parse(tmp, length, &remaining, &value);
	
	printf("start  = 0x%p\n", tmp);
	printf("length = %u\n", length);
	printf("end    = 0x%p\n", end);
	printf("remaining = %u\n", remaining);

	printf("type = %c\n", value->type);
	
	if (value->type == DICTIONARY) {
		bencoded_dict *dict = (bencoded_dict *) value;
		for (auto e : *dict) {
			
			//printf("%p: ", e.first);
			for(unsigned int i=0; i<e.first.length; i++)
				printf("%c", e.first.byte_str[i]);
			printf("\n");

			printf(" : %p", e.second);
			printf("\n\n");
		}
	}
	
	*/
	
	bittorrent_talk_with_peer();//TODO
	//exit(0);
	
//	thread first(foo);
	
	
	// Read in torrent file
	bencoded_dict torrent_dict;
	if (!read_bencoded_dict_file("./[limetorrents.io]Windows.7.SP1.x86x64.18in1.OEM.ESD.hu-HU.June.2016.{Gen2}.torrent", &torrent_dict)) {

		printf("failed to read in torrent file\n");
		exit(EXIT_FAILURE);
	}
	
	//
	torrent = bittorrent::torrent_file(torrent_dict);
	

	// Seed random
	srand((unsigned)time(NULL));

	// Initialize starting transaction ID
	next_transaction_id = rand();

	//
	generate_peer_id();


	//TODO
	

	connect_to_tracker_udp();



	/*
	printf("type = %c\n", torrent.type);

	if (torrent.type == DICTIONARY) {

		for (auto e : torrent) {

			//printf("%p: ", e.first);
			for (unsigned int i = 0; i<e.first.length; i++)
				printf("%c", e.first.byte_str[i]);
			printf("\n");

			printf(" : %p", e.second);
			printf("\n");
		}
	}

	*/

	/*
	size_t bencoded_torrent_file_size = 0;
	unsigned char *bencoded_torrent_file = torrent.bencoding(&bencoded_torrent_file_size);
	printf("bencoded size = %u\n", bencoded_torrent_file_size);
	for (size_t i = 0; i < bencoded_torrent_file_size; i++) {
		printf("%c", bencoded_torrent_file[i]);
	}
	free(bencoded_torrent_file);
	*/
	
	

//	first.join();


#if defined(_WIN32)
	if (WSACleanup()) {
		WSAGetLastError(); // TODO
	}
#endif


	//system("pause");
	return EXIT_SUCCESS;
}