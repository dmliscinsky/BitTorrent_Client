/*
 * Author: Daniel Liscinsky
 */

#pragma once

#ifndef _BITTORRENT_H
#define _BITTORRENT_H


#if defined(_WIN32)
//#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#else
//#include <arpa/inet.h>
#include <netdb.h>
#endif

#include <stdint.h>

#include "inet_cross_platform.h"
#include "bencode.h"



namespace bittorrent {

using namespace bencode;


#define MAX_PORT_STRING_LEN 5

#define BITTORRENT_PEER_ID_LEN 20

#ifndef SHA1_HASH_LENGTH
#define SHA1_HASH_LENGTH 20
#endif



const bencoded_byte_str bencoded_str_announce		("announce", 8);
const bencoded_byte_str bencoded_str_announce_list	("announce-list", 13);
const bencoded_byte_str bencoded_str_comment		("comment", 7);
const bencoded_byte_str bencoded_str_created_by		("created by", 10);
const bencoded_byte_str bencoded_str_creation_date	("creation date", 13);
const bencoded_byte_str bencoded_str_encoding		("encoding", 8);
const bencoded_byte_str bencoded_str_info			("info", 4);
const bencoded_byte_str bencoded_str_files			("files", 5);
const bencoded_byte_str bencoded_str_length			("length", 6);
const bencoded_byte_str bencoded_str_path			("path", 4);
const bencoded_byte_str bencoded_str_name			("name", 4);
const bencoded_byte_str bencoded_str_piece			("piece", 5);
const bencoded_byte_str bencoded_str_pieces			("pieces", 6);
const bencoded_byte_str bencoded_str_piece_length	("piece length", 12);



/**
 * 
 */
struct tracker {
	std::string uri;
	std::string host;
	char port[MAX_PORT_STRING_LEN + 1]; // A null terminated string representing the port nubmer

	int socktype; // SOCK_xxx		(i.e. SOCK_STREAM, SOCK_DGRAM, ...)
	struct addrinfo *addr;
};

/**
*
*/
struct peer {

	int choked; // state (1 bit)
	int interested; // state (1 bit)

	//struct sock_addr;

};



/**
 * 
 */
class torrent_file {

public:
	struct tracker announcer;
	std::vector<struct tracker> announcer_list;

	bencoded_byte_str comment;
	bencoded_byte_str created_by;
	bencoded_byte_str creation_date;
	bencoded_byte_str encoding;

	uint8_t info_hash[SHA1_HASH_LENGTH];
	std::string info_hash_str; // The info hash, as a percent encoded string

private:
	bencoded_dict info;

public:
	torrent_file();
	torrent_file(const bencoded_dict& dict);

private:
	bool compute_info_hash();
};



#define UDP_TRACKER_PROTOCOL_ID 0x41727101980 // magic constant
#define UDP_TRACKER_ACTION_CONNECT		0
#define UDP_TRACKER_ACTION_ANNOUNCE		1



/**
 * UDP tracker protocol
 * Request / 
 * 
 */
PACKED(
struct tracker_connect_req_msg {
	uint64_t protocol_id;		// 0x41727101980
	uint32_t action;			// action = 0
	uint32_t transaction_id;
});

/**
 * UDP tracker protocol
 * 
 * 
 */
PACKED(
struct tracker_connect_resp_msg {
	uint32_t action;			// action = 0
	uint32_t transaction_id;
	uint64_t connection_id;
});

/**
 * UDP tracker protocol
 * Announce Request
 * 
 */
PACKED(
struct tracker_announce_req_msg {
	uint64_t connection_id;
	uint32_t action;			// action = 1
	uint32_t transaction_id;
	
	uint8_t info_hash[SHA1_HASH_LENGTH];
	uint8_t peer_id[BITTORRENT_PEER_ID_LEN];

	uint64_t downloaded;
	uint64_t left;
	uint64_t uploaded;
	uint32_t event;				// 0: none; 1: completed; 2: started; 3: stopped
	uint32_t ip_address;		// 0 default
	uint32_t key;
	uint32_t num_want;			// -1 default
	uint16_t port;
});

/**
 * UDP tracker protocol
 * Announce Response
 * 
 * 
 */
PACKED(
struct tracker_announce_resp_msg {
	uint32_t action;			// action = 1
	uint32_t transaction_id;

	uint32_t interval;
	uint32_t leechers;
	uint32_t seeders;
	
	uint8_t ip_port_list[];
});



#define BITTORRENT_MSG_TYPE_UNCHOKE		1
#define BITTORRENT_MSG_TYPE_INTERESTED	2
#define BITTORRENT_MSG_TYPE_HAVE		4
#define BITTORRENT_MSG_TYPE_BITFIELD	5
#define BITTORRENT_MSG_TYPE_REQUEST		6
#define BITTORRENT_MSG_TYPE_EXTENDED	20



/**
 * Bittorrent protocol
 * Handshake message
 * 
 * 
 */
PACKED(
struct bittorrent_handshake {
	uint8_t proto_name_len = 19;			// This field is always 19 bytes for the Bittorrent protocol
	uint8_t proto_name[19];					// "Bittorrent protocol"		// This variable length field is always 19 bytes for the Bittorrent protocol
	uint64_t reserved_extension;
	uint8_t info_hash[SHA1_HASH_LENGTH];
	uint8_t peer_id[BITTORRENT_PEER_ID_LEN];
});

/**
 * Bittorrent protocol
 * Message Header
 * 
 * 
 */
PACKED(
struct bittorrent_msg_hdr {
	uint32_t length;
	uint8_t type;
});

/**
 * Bittorrent protocol
 * Have Message
 * 
 * 
 */
PACKED(
struct bittorrent_msg_have {
	struct bittorrent_msg_hdr hdr;
	uint32_t piece_index;
});

/**
 * Bittorrent protocol
 * Bitfield Message
 * 
 * 
 */
PACKED(
struct bittorrent_msg_bitfield {
	struct bittorrent_msg_hdr hdr;
	uint8_t bitfield[];
});

/**
 * Bittorrent protocol
 * Request Message
 * 
 * 
 */
PACKED(
struct bittorrent_msg_req {
	struct bittorrent_msg_hdr hdr;
	uint32_t piece_index;
	uint32_t begin_offset;
	uint32_t piece_length;
});

/**
 * Bittorrent protocol
 * Extended Message
 * 
 * 
 */
PACKED(
struct bittorrent_msg_extended {
	struct bittorrent_msg_hdr hdr;
	uint8_t data[];
});


}


#endif