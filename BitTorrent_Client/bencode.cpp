/*
 * Author: Daniel Liscinsky
 */

#include "bencode.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <new>
#include <string>


using namespace std;
using namespace bencode;



#define BUFFER_RESIZE_FACTOR 2

#define min(X, Y)  ((X) < (Y) ? (X) : (Y))



//--------------------------------------------------------
//					bencoded_value
//--------------------------------------------------------

unsigned char * bencoded_value::bencoding(size_t *size_out) const {

	unsigned char *bencoded_out = NULL;
	size_t size = 0;
	size_t offset = 0;

	// Bencode the data
	if (!internal_bencode(&bencoded_out, &size, &offset)) {
		return NULL;
	}

	*size_out = offset;
	return bencoded_out;
}



//--------------------------------------------------------
//					bencoded_int
//--------------------------------------------------------

bool bencoded_int::internal_bencode(unsigned char **_bencoded_out, size_t *_size, size_t *_offset) const {

	unsigned char *bencoded_out = *_bencoded_out;
	size_t size = *_size;
	size_t offset = *_offset;

	string value_str = std::to_string(this->value); // Convert value to string
	size_t data_size = 2 + value_str.length();


	// If size of data to write is greater than space remaining in buffer
	if (data_size > size - offset) {

		// If buffer is unallocated, make size exactly amount needed
		if (!size) {
			size = data_size;
		}
		else {
			// Increase size until it is sufficient
			do {
				size *= BUFFER_RESIZE_FACTOR;
			} while (data_size > size - offset);
		}

		// Reallocate a bigger buffer
		unsigned char *new_bencoded_out = (unsigned char *)realloc(bencoded_out, size);
		if (!new_bencoded_out) {
			return false;
		}

		// Update the buffer parameters
		*_bencoded_out = bencoded_out = new_bencoded_out;
		*_size = size;
	}


	// Integer start delimiter
	*(*_bencoded_out + offset++) = BENCODE_INTEGER_DELIMITER;

	// Copy the integer string into buffer
	memcpy(bencoded_out + offset, value_str.data(), value_str.length());
	offset += value_str.length();

	// Integer end delimiter
	*(*_bencoded_out + offset++) = BENCODE_TERMINATOR_BYTE;

	// Update offset
	*_offset = offset;

	return true;
}



//--------------------------------------------------------
//					bencoded_byte_str
//--------------------------------------------------------

bencoded_byte_str::bencoded_byte_str() : bencoded_value(BYTE_STRING) {

	byte_str = NULL;
	length = 0;

	_byte_str_ptr = shared_ptr<char>(byte_str, free);
}

bencoded_byte_str::bencoded_byte_str(const char *data, unsigned int length) : bencoded_value(BYTE_STRING) {

	this->length = length;

	if (length) {
		byte_str = (char *)malloc(length);
		if (!byte_str) {
			throw new std::bad_alloc;
		}

		memcpy(byte_str, data, length);
	}
	else {
		byte_str = NULL;
	}

	_byte_str_ptr = shared_ptr<char>(byte_str, free);
}

bencoded_byte_str::~bencoded_byte_str() {
}

bool bencoded_byte_str::internal_bencode(unsigned char **_bencoded_out, size_t *_size, size_t *_offset) const {
	
	unsigned char *bencoded_out = *_bencoded_out;
	size_t size = *_size;
	size_t offset = *_offset;
	
	string length_str = std::to_string(this->length); // Convert byte str length to string
	size_t data_size = length_str.length() + sizeof(':') + this->length;
	

	// If size of data to write is greater than space remaining in buffer
	if (data_size > size - offset) {

		// If buffer is unallocated, make size exactly amount needed
		if (!size) {
			size = data_size;
		}
		else {
			// Increase size until it is sufficient
			do {
				size *= BUFFER_RESIZE_FACTOR;
			} while (data_size > size - offset);
		}

		// Reallocate a bigger buffer
		unsigned char *new_bencoded_out = (unsigned char *)realloc(bencoded_out, size);
		if (!new_bencoded_out) {
			return false;
		}

		// Update the buffer parameters
		*_bencoded_out = bencoded_out = new_bencoded_out;
		*_size = size;
	}


	// Copy byte string length into buffer
	memcpy(bencoded_out + offset, length_str.c_str(), length_str.length());
	offset += length_str.length();

	// Copy separator into buffer
	*(bencoded_out + offset++) = ':';

	// Copy byte string data into buffer
	memcpy(bencoded_out + offset, this->byte_str, this->length);
	offset += this->length;

	// Update offset
	*_offset = offset;

	return true;
}

bool bencoded_byte_str::operator == (const bencoded_byte_str& other) const  {

	if (this->length != other.length) {
		return false;
	}

	return 0 == memcmp(this->byte_str, other.byte_str, this->length);
}

bool bencoded_byte_str::operator <= (const bencoded_byte_str& other) const {

	int cmp = memcmp(this->byte_str, other.byte_str, min(this->length, other.length));
	
	if (cmp < 0) {
		return true;
	}
	else if (cmp == 0) {
		return this->length <= other.length;
	}
	else {
		return false;
	}
}

bool bencoded_byte_str::operator < (const bencoded_byte_str& other) const {

	int cmp = memcmp(this->byte_str, other.byte_str, min(this->length, other.length));

	if (cmp < 0) {
		return true;
	}
	else if (cmp == 0) {
		return this->length < other.length;
	}
	else {
		return false;
	}
}

bool bencoded_byte_str::operator >= (const bencoded_byte_str& other) const {

	int cmp = memcmp(this->byte_str, other.byte_str, min(this->length, other.length));

	if (cmp > 0) {
		return true;
	}
	else if (cmp == 0) {
		return this->length >= other.length;
	}
	else {
		return false;
	}
}

bool bencoded_byte_str::operator > (const bencoded_byte_str& other) const {

	int cmp = memcmp(this->byte_str, other.byte_str, min(this->length, other.length));

	if (cmp > 0) {
		return true;
	}
	else if (cmp == 0) {
		return this->length > other.length;
	}
	else {
		return false;
	}
}



//--------------------------------------------------------
//					bencoded_list
//--------------------------------------------------------

bool bencoded_list::internal_bencode(unsigned char **_bencoded_out, size_t *_size, size_t *_offset) const {

	unsigned char *bencoded_out = *_bencoded_out;
	size_t size = *_size;
	size_t offset = *_offset;
	
	size_t data_size_min = 2 + this->size() * sizeof(bencoded_value); // The min size in bytes needed to encoded this list (true size will likely be larger)


	// If size of data to write is greater than space remaining in buffer
	if (data_size_min > size - offset) {

		// If buffer is unallocated, make size approximately amount needed
		if (!size) {
			size = data_size_min;
		}
		else {
			// Increase size until it is sufficient
			do {
				size *= BUFFER_RESIZE_FACTOR;
			} while (data_size_min > size - offset);
		}

		// Reallocate a bigger buffer
		unsigned char *new_bencoded_out = (unsigned char *)realloc(bencoded_out, size);
		if (!new_bencoded_out) {
			return false;
		}

		// Update the buffer parameters
		*_bencoded_out = new_bencoded_out;
		*_size = size;
	}


	// List start delimiter
	*(*_bencoded_out + (*_offset)++) = BENCODE_LIST_DELIMITER;
	
	// Bencode each element in the list
	for (bencoded_value *elem : *this) {

		if (!elem->internal_bencode(_bencoded_out, _size, _offset)) {
			return false;
		}
	}
	
	// List end delimiter
	*(*_bencoded_out + (*_offset)++) = BENCODE_TERMINATOR_BYTE;

	return true;
}



//--------------------------------------------------------
//					bencoded_dict
//--------------------------------------------------------

bool bencoded_dict::internal_bencode(unsigned char **_bencoded_out, size_t *_size, size_t *_offset) const {

	unsigned char *bencoded_out = *_bencoded_out;
	size_t size = *_size;
	size_t offset = *_offset;

	size_t data_size_min = 2 + this->size() * sizeof(bencoded_value); // The min size in bytes needed to encoded this dict (true size will likely be larger)


	// If size of data to write is greater than space remaining in buffer
	if (data_size_min > size - offset) {

		// If buffer is unallocated, make size approximately amount needed
		if (!size) {
			size = data_size_min;
		}
		else {
			// Increase size until it is sufficient
			do {
				size *= BUFFER_RESIZE_FACTOR;
			} while (data_size_min > size - offset);
		}

		// Reallocate a bigger buffer
		unsigned char *new_bencoded_out = (unsigned char *)realloc(bencoded_out, size);
		if (!new_bencoded_out) {
			return false;
		}

		// Update the buffer parameters
		*_bencoded_out = new_bencoded_out;
		*_size = size;
	}


	// Dictionary start delimiter
	*(*_bencoded_out + (*_offset)++) = BENCODE_DICTIONARY_DELIMITER;

	// Bencode each key value pair in the dictionary
	for (auto elem : *this) {

		if (!elem.first.internal_bencode(_bencoded_out, _size, _offset)) {
			return false;
		}

		if (!elem.second->internal_bencode(_bencoded_out, _size, _offset)) {
			return false;
		}
	}

	// Dictionary end delimiter
	*(*_bencoded_out + (*_offset)++) = BENCODE_TERMINATOR_BYTE;

	return true;
}



//--------------------------------------------------------
//				bencode parsing functions
//--------------------------------------------------------

/**
 * 
 */
static void bencode_parser_error(const char *msg) {
	fprintf(stderr, "Bencode Parser: %s\n", msg);
}

/**
 * Whitespace characters incldue ' ', '\t', '\v', '\r', '\n'.
 * 
 * @retrun Non-zero if character represents a whitespace character. Otherwise, returns zero.
 */
static int iswhitesapce(char c) {
	return c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\v';
}



char * bencode::bencode_parse(char *bencode_data, size_t length, size_t *length_remaining, bencoded_value **value) {

	// If data is empty
	if (!length) {
		return NULL;
	}

	switch (*bencode_data) {
		
	case BENCODE_INTEGER_DELIMITER:
		//*value = (bencoded_value *)calloc(1, sizeof(bencoded_int));
		*value = new (nothrow) bencoded_int(0);
		if (!*value) {
			return NULL;
		}
		bencode_data = bencode_parse_integer(bencode_data, length, length_remaining, *(bencoded_int **)value);
		break;
		
	case BENCODE_LIST_DELIMITER:
		//*value = (bencoded_value *)calloc(1, sizeof(bencoded_list));
		*value = new (nothrow) bencoded_list;
		if (!*value) {
			return NULL;
		}
		bencode_data = bencode_parse_list(bencode_data, length, length_remaining, *(bencoded_list **)value);
		break;
		
	case BENCODE_DICTIONARY_DELIMITER:
		//*value = (bencoded_type *)calloc(1, sizeof(bencoded_dict));
		*value = new (nothrow) bencoded_dict;
		if (!*value) {
			return NULL;
		}
		bencode_data = bencode_parse_dict(bencode_data, length, length_remaining, *(bencoded_dict **)value);
		break;

		// Otherwise must be a bencoded byte string
	default:
		//*value = (bencoded_type *)calloc(1, sizeof(bencoded_byte_str));
		*value = new (nothrow) bencoded_byte_str;
		if (!*value) {
			return NULL;
		}
		bencode_data = bencode_parse_byte_string(bencode_data, length, length_remaining, *(bencoded_byte_str **)value);
		break;
	}

	// Clean up if parsing failed
	if (!bencode_data) {
		free(*value);
		*value = NULL;
	}

	return bencode_data;
}

char * bencode::bencode_parse_integer(char *bencode_data, size_t length, size_t *length_remaining, bencoded_int *integer) {

	// If cannot possibly be a valid int
	if (length < 3) {
		return NULL;
	}

	// Ensure first byte is int delimiter
	if (bencode_data[0] != BENCODE_INTEGER_DELIMITER) {
		bencode_parser_error("int does not start with int delimiter");
		return NULL;
	}
	
	// Skip over start delimiter
	bencode_data++;
	length--;

	// Do not allow leading whitespace
	if (iswhitesapce(bencode_data[0])) {
		bencode_parser_error("int has leading whitespace");
		return NULL;
	}

	// Find index of terminating delimiter
	unsigned int idx = 0;
	while (idx < length && bencode_data[idx] != BENCODE_TERMINATOR_BYTE) {
		idx++;
	}

	// If the number or terminating delimiter is missing
	if (!idx || idx == length) {
		bencode_parser_error("int number or terminating delimiter is missing");
		return NULL;
	}

	// Replace terminating delimiter with null byte to make number into a C string
	bencode_data[idx] = '\0';

	char *end_ptr = NULL;
	long long number = strtoll(bencode_data, &end_ptr, 10);

	// If an overflow or underflow occurred, or the entire number string was not valid
	if (errno == ERANGE || end_ptr != bencode_data + idx) {
		bencode_parser_error("int string not a valid number");
		return NULL;
	}
	
	// 
	*integer = bencoded_int(number);
	*length_remaining = length - (idx + 1);
	return bencode_data + idx + 1;
}

char * bencode::bencode_parse_byte_string(char *bencode_data, size_t length, size_t *length_remaining, bencoded_byte_str *byte_str) {

	// If cannot possibly be a valid byte string
	if (length < 3) {
		return NULL;
	}
	
	// Do not allow leading whitespace
	if (iswhitesapce(bencode_data[0])) {
		bencode_parser_error("byte string has leading whitespace");
		return NULL;
	}

	// Find index of ':' separator
	unsigned int idx = 0;
	while (idx < length && bencode_data[idx] != ':') {
		idx++;
	}

	// If leading length number or ':' separator is missing
	if (!idx || idx == length) {
		bencode_parser_error("byte string leading length number or ':' separator is missing");
		return NULL;
	}

	// Replace ':' with null byte to make length number into a C string
	bencode_data[idx] = '\0';
	
	char *end_ptr = NULL;
	unsigned long byte_str_length = strtoul(bencode_data, &end_ptr, 10);

	// If an overflow or underflow occurred, or the entire number string was not valid
	if (errno == ERANGE || end_ptr != bencode_data + idx) {
		bencode_parser_error("byte string length not a valid number");
		return NULL;
	}

	// Advance to byte string value
	idx++;
	bencode_data += idx;
	length -= idx;

	// If claiming more bytes in string than there are left in the data buffer
	if (byte_str_length > length) {
		bencode_parser_error("byte string length is greater than the bytes left in the data buffer");
		return NULL;
	}
	
	// 
	try {
		*byte_str = bencoded_byte_str(bencode_data, byte_str_length);
	}
	catch (std::bad_alloc& ex) {
		ex; // To satisfy compiler
		return NULL;
	}

	*length_remaining = length - byte_str_length;
	return bencode_data + byte_str_length;
}

char * bencode::bencode_parse_list(char *bencode_data, size_t length, size_t *length_remaining, bencoded_list *_list) {

	bencoded_list list;

	// If cannot possibly be a valid list
	if (length < 2) {
		return NULL;
	}
	
	// Ensure first byte is list delimiter
	if (bencode_data[0] != BENCODE_LIST_DELIMITER) {
		bencode_parser_error("list does not start with list delimiter");
		return NULL;
	}

	// Skip over start delimiter
	bencode_data++;
	length--;

	// While did not reach ending byte terminator delimiter
	while (length > 0 && bencode_data[0] != BENCODE_TERMINATOR_BYTE) {
		
		// Parse the value (could be any type)
		bencoded_value *value = NULL;
		bencode_data = bencode_parse(bencode_data, length, &length, &value);
		if (!bencode_data) {
			bencode_parser_error("list failed to parse value");
			return NULL;
		}

		// Insert value into list
		list.push_back(value);
	}
	
	// If ending byte terminator delimiter is not present
	if (!length) {
		bencode_parser_error("list does not end with terminating delimiter");
		return NULL;
	}


	*_list = list;
	*length_remaining = length - 1;
	return bencode_data + 1;
}

char * bencode::bencode_parse_dict(char *bencode_data, size_t length, size_t *length_remaining, bencoded_dict *_dict) {

	bencoded_dict dict;
	bencoded_byte_str prev_key;

	// If cannot possibly be a valid dictionary
	if (length < 2) {
		return NULL;
	}
	
	// Ensure first byte is dictionary delimiter
	if (bencode_data[0] != BENCODE_DICTIONARY_DELIMITER) {
		bencode_parser_error("dictionary does not start with dictionary delimiter");
		return NULL;
	}

	// Skip over start delimiter
	bencode_data++;
	length--;

	// While did not reach ending byte terminator delimiter
	while (length > 0 && bencode_data[0] != BENCODE_TERMINATOR_BYTE) {
		
		// Parse the byte string key
		bencoded_byte_str key;
		bencode_data = bencode_parse_byte_string(bencode_data, length, &length, &key);
		if (!bencode_data) {
			bencode_parser_error("dictionary failed to parse byte string key");
			return NULL;
		}

		// Ensure lexicographic ordering of keys and non duplicate keys
		if (key <= prev_key) {

			// There is the (allowed) case in which the first key has zero length,
			// key == prev_key on the first iteration, otherwise declare an error parsing
			if (dict.size() > 0) {
				bencode_parser_error("dictionary has non-lexicographic ordering or a duplicate key");
				return NULL;
			}
		}
		
		// Parse the value (could be any type)
		bencoded_value *value = NULL;
		bencode_data = bencode_parse(bencode_data, length, &length, &value);
		if (!bencode_data) {
			bencode_parser_error("dictionary failed to parse value for key");
			return NULL;
		}

		// Insert key value pair into dictionary
		dict.emplace_hint(dict.end(), key, value);
		prev_key = key;
	}
	
	// If ending byte terminator delimiter is not present
	if (!length) {
		bencode_parser_error("dictionary does not end with terminating delimiter");
		return NULL;
	}


	*_dict = dict;
	*length_remaining = length - 1;
	return bencode_data + 1;
}