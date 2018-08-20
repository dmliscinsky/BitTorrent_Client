/*
 * Author: Daniel Liscinsky
 */

#if defined(_WIN32)
#include "memutil.h"
#endif

#include "bittorrent.h"
#include "sha1.h"

#include <ctype.h>
#include <string.h>

#include <stdexcept>


using namespace std;
using namespace bencode;
using namespace bittorrent;



string percent_encode(string str) {

	size_t len = str.length();
	string escaped_str;

	char percent[4] = { 0 };

	// Percent encode the string
	for (size_t i = 0; i < len; i++) {

		// If character is alphanumeric, keep as is
		if (isalnum(str[i] & 0xff)) {
			escaped_str += str[i];
		}
		// Else percent encode byte
		else {
			snprintf(percent, sizeof(percent), "%%%02hhx", str[i]);
			escaped_str += percent;
		}
	}

	return escaped_str;
}

/**
 *
 */
const char * get_port_from_scheme(const char *scheme, size_t scheme_len) {

	// If scheme is http
	if (scheme_len == 4 && *(uint32_t *)scheme == *(uint32_t *)"http") {
		return "80";
	}

	return NULL;
}

/**
 * Initializes a tracker struct based on a URL, except for the addrinfo field.
 * 
 * @param url			The URL string to use to initialize the struct. Does not have 
 *						to be null terminated.
 * @param url_length	The length of the URL in bytes, not including any null 
 *						terminating byte if one is present.
 * 
 * @return Zero on failure, and non-zero on success.
 */
int init_tracker(struct bittorrent::tracker *tracker, const char *url, size_t url_length) {

	/*
	The following is mostly generalized to parsing any URL. The only specific part about it
	is how the data is stored once it is parsed out, and some default values for optional 
	URL components that are not present.
	*/

	/*
	Note: memchr()/memmem() is used in place of strchr()/strstr() because none of the byte
	strings or sub strings are null terminated, however this does not really impact the code.
	*/
	
	// Initialize struct to be empty
	tracker->uri = string();
	tracker->host = string();
	memset(tracker->port, 0, MAX_PORT_STRING_LEN + 1);
	tracker->addr = NULL;


	// Parse URL
	const char *scheme = url;
	const char *authority = (char *)memmem((void *)scheme, url_length, "://", 3);

	size_t scheme_len = authority - scheme;
	size_t authority_len = url_length - scheme_len - strlen("://");
	size_t abs_path_len = 0;

	authority = authority + 3; // Skip past "://"
	const char *abs_path = (char *)memchr(authority, '/', authority_len);

	// If abs_path is NOT present
	if (!abs_path) {

		// Store the default abs_path
		tracker->uri = string("/");
	}
	// Else abs_path is present
	else {

		// Update length of authority & abs_path portions of URL
		authority_len = abs_path - authority;
		abs_path_len = url_length - (abs_path - url);

		// Store the abs_path
		tracker->uri = string(abs_path, abs_path_len);
	}


	// 
	// Always assume authority is server
	// 

	// Initially assume there is no port specified in server 
	const char *userinfo_host = authority;
	size_t userinfo_host_len = authority_len;

	// Check if port is explicitly specified in server
	const char *port = (char *)memchr(authority, ':', authority_len);

	// If port is present
	if (port) {

		// Update length of userinfo/host portion of server
		userinfo_host_len = port - authority;

		// Store port string
		size_t port_len = authority_len - userinfo_host_len - 1;
		memcpy(tracker->port, ++port, port_len);
	}
	// Else port is NOT explicitly specified in URL
	else {

		// Must determine standard port from scheme
		port = get_port_from_scheme(scheme, scheme_len);

		// Store port string
		strncpy(tracker->port, port, MAX_PORT_STRING_LEN);
	}
	tracker->port[MAX_PORT_STRING_LEN] = '\0'; // Always ensure null terminated


	// Check if userinfo is specified in server
	const char *userinfo = userinfo_host;
	const char *host = (char *)memchr(userinfo_host, '@', userinfo_host_len);

	size_t userinfo_len = 0;
	size_t host_len = userinfo_host_len;

	// If userinfo is present
	if (host) {

		// Update length of individual userinfo & host portions
		userinfo_len = host - userinfo_host;
		host_len = userinfo_host_len - userinfo_len - 1;

		host++; // Skip past "@"

		//TODO ????
	}
	// Else userinfo is NOT present
	else {
		userinfo = NULL;
		host = userinfo_host;
	}


	// Store host string
	tracker->host = string(host, host_len);

	// Determine the socktype
	tracker->socktype = scheme_len == 3 && !memcmp(scheme, "udp", 3) ? SOCK_DGRAM : SOCK_STREAM;

	return 1;
}



//--------------------------------------------------------
//					torrent_file
//--------------------------------------------------------

torrent_file::torrent_file() {
	
	//TODO
}

torrent_file::torrent_file(const bencoded_dict& dict) {

	// If required key is not present
	auto announce_keyval = dict.find(bencoded_str_announce);
	if (announce_keyval == dict.end() || announce_keyval->second->type != BYTE_STRING) {
		throw std::invalid_argument("torrent_file: bencoded_dict does not contain 'announce' key or value is not bencoded_byte_str");
	}

	// If required key is not present or has the wrong value
	auto info_keyval = dict.find(bencoded_str_info);
	if (info_keyval == dict.end() || info_keyval->second->type != DICTIONARY){
		throw std::invalid_argument("torrent_file: bencoded_dict does not contain 'info' key or value is not bencoded_dict");
	}

	// Store values
	bencoded_byte_str *announce = (bencoded_byte_str *)announce_keyval->second;
	info = *(bencoded_dict *)info_keyval->second;
	
	// Parse announce URL
	if (!init_tracker(&announcer, announce->byte_str, announce->length)) {
		throw std::invalid_argument("torrent_file: failed to parse announce URL");
	}
	
	// Compute hash of info value
	if (!compute_info_hash()) {
		throw runtime_error("torrent_file: failed to bencode 'info' value");
	}


	// Check if 'announce-list' key is present
	announce_keyval = dict.find(bencoded_str_announce_list);
	if (announce_keyval != dict.end()) {
		if (announce_keyval->second->type != LIST) {
			throw std::invalid_argument("torrent_file: 'announce-list' value is not bencoded_list");
		}


		//TODO ensure 'announce' tracker is added as the first item in the list (in the case where the 'announce' tracker was not present in the 'announce-list')
		

		// Add each tracker to the list
		struct tracker tracker;
		bencoded_list tracker_list = *(bencoded_list *)announce_keyval->second;
		for(auto e : tracker_list) {
			
			// Announce value is wrapped in a list of one item for some reason
			if (e->type == LIST) {
				announce = (bencoded_byte_str *)((bencoded_list *)e)->at(0);

				// If list value does not meet expected format
				if (announce->type != BYTE_STRING || ((bencoded_list *)e)->size() != 1) {
					throw std::invalid_argument("torrent_file: invalid 'announce-list' value");
				}
			}
			// Announce value is directly in list
			else if(e->type == BYTE_STRING) {
				announce = (bencoded_byte_str *)e;
			}
			// Else unexpected element type
			else {
				throw std::invalid_argument("torrent_file: invalid 'announce-list' value unexpected bencode type");
			}

			
			// Store tracker info
			if (!init_tracker(&tracker, announce->byte_str, announce->length)) {
				throw std::invalid_argument("torrent_file: failed to parse URL in 'announce-list'");
			}
			announcer_list.push_back(tracker);
		}

	}
	// Else add announcer as the only item in the announce_list
	else {
		announcer_list.push_back(announcer);
	}

	
	// Save other optional key values
	auto other_keyval = dict.find(bencoded_str_comment);
	if (other_keyval != dict.end() && other_keyval->second->type == BYTE_STRING) {
		comment = *(bencoded_byte_str *)other_keyval->second;
	}

	other_keyval = dict.find(bencoded_str_created_by);
	if (other_keyval != dict.end() && other_keyval->second->type == BYTE_STRING) {
		created_by = *(bencoded_byte_str *)other_keyval->second;
	}

	other_keyval = dict.find(bencoded_str_creation_date);
	if (other_keyval != dict.end() && other_keyval->second->type == BYTE_STRING) {
		creation_date = *(bencoded_byte_str *)other_keyval->second;
	}

	other_keyval = dict.find(bencoded_str_encoding);
	if (other_keyval != dict.end() && other_keyval->second->type == BYTE_STRING) {
		encoding = *(bencoded_byte_str *)other_keyval->second;
	}

}

bool torrent_file::compute_info_hash() {

	SHA1_CTX ctx = { 0 };
	memset(info_hash, 0, SHA1_HASH_LENGTH);

	// 
	size_t bencoded_info_len = 0;
	unsigned char * bencoded_info = info.bencoding(&bencoded_info_len);
	if (!bencoded_info) {
		return false;
	}
	
	// Take the SHA1 hash of the info value
	SHA1Init(&ctx);
	SHA1Update(&ctx, bencoded_info, bencoded_info_len);
	SHA1Final(info_hash, &ctx);
	
	// Free memory
	free(bencoded_info);

	// Percent encode as necessary
	info_hash_str = percent_encode( string((char *)info_hash, SHA1_HASH_LENGTH) );
	
	return true;
}