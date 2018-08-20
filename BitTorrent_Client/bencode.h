/*
 * Author: Daniel Liscinsky
 */

#pragma once

#ifndef _BENCODE_H
#define _BENCODE_H

#include <memory>
#include <vector>
#include <map>



namespace bencode {


#define BENCODE_INTEGER_DELIMITER 'i'
#define BENCODE_LIST_DELIMITER 'l'
#define BENCODE_DICTIONARY_DELIMITER 'd'
#define BENCODE_TERMINATOR_BYTE 'e'



/**
 * 
 */
enum bencode_type {
	BYTE_STRING,
	INTEGER = 'i',
	LIST = 'l',
	DICTIONARY = 'd',
};

/**
 * 
 */
class bencoded_value {
	friend class bencoded_int;
	friend class bencoded_byte_str;
	friend class bencoded_list;
	friend class bencoded_dict;

public:
	bencode_type type;
	
	bencoded_value(bencode_type type) : type(type) {}

	/**
	 * 
	 * Note: You must free the returned pointer using free().
	 * 
	 * 
	 */
	virtual unsigned char * bencoding(size_t *size_out) const;

protected:

	/**
	 *
	 * 
	 * bencode_data			The bencoded data bytes to be parsed.
	 * size					The length of bencoded_out in bytes. Specifically, the number of bytes from bencoded_out 
	 *						until the end of the allocated region.
	 * size_remaining		Out param. 
	 *
	 * @retrun NULL on failure. On success, returns a pointer to the next byte following the written bencoded bytes.
	 */
	virtual bool internal_bencode(unsigned char **_bencoded_out, size_t *_size, size_t *_offset) const = 0;
};


/**
 *
 */
class bencoded_int : public bencoded_value {

public:
	long long value;

public:
	bencoded_int(long long i) : value(i) , bencoded_value(INTEGER) {}
	
protected:
	bool internal_bencode(unsigned char **_bencoded_out, size_t *_size, size_t *_offset) const;
};


/**
 * 
 * 
 * Note: You cannout have a bencoded_byte_str with a length greater than UINT32_MAX (0xffffffff).
 * 
 *
 * TODO REMOVE WARNING (it is not an issue anymore)
 * 
 * Warning: Using comparison operators (<, >, ==, etc.) on uninitialized objects 
 * of this class may cause an access violation (i.e. NULL pointer dereference) 
 * because no checking is performed to determine whether the byte_str pointer is 
 * valid.
 */
class bencoded_byte_str : public bencoded_value {
	friend class bencoded_dict;

private:
	std::shared_ptr<char> _byte_str_ptr;

public:
	char *byte_str;
	unsigned int length;

public:
	bencoded_byte_str();
	bencoded_byte_str(const char *data, unsigned int length);
	~bencoded_byte_str();
	
protected:
	bool internal_bencode(unsigned char **_bencoded_out, size_t *_size, size_t *_offset) const;
	
public:
	bool operator == (const bencoded_byte_str&) const;
	bool operator <= (const bencoded_byte_str&) const;
	bool operator < (const bencoded_byte_str&) const;
	bool operator >= (const bencoded_byte_str&) const;
	bool operator > (const bencoded_byte_str&) const;
};


/**
 * 
 */
class bencoded_list : public bencoded_value, public std::vector<bencoded_value *> {

public:
	bencoded_list() : bencoded_value(LIST) {}
	/*
	bencoded_list& operator = (const bencoded_list& o) {
		this->vector::operator=(o);
		return *this;
	};
	*/
	
protected:
	bool internal_bencode(unsigned char **_bencoded_out, size_t *_size, size_t *_offset) const;
};


/**
 * 
 */
class bencoded_dict : public bencoded_value, public std::map<bencoded_byte_str, bencoded_value *> {

public:
	bencoded_dict() : bencoded_value(DICTIONARY) {}
	/*
	bencoded_dict& operator = (const bencoded_dict& o) {
		this->map::operator=(o);
		return *this;
	};
	*/
	
protected:
	bool internal_bencode(unsigned char **_bencoded_out, size_t *_size, size_t *_offset) const;
};






//--------------------------------------------------------
//				bencode parsing functions
//--------------------------------------------------------



/**
 * 
 * bencode_data			The bencoded data bytes to be parsed.
 * length				The length of bencode_data in bytes.
 * length_remaining		Out param. The number of bytes remaining in bencode_data after parsing out the bencoded value.
 * bencoded_value		Out param. The parsed bencoded value.
 * 
 * Note: This fucntion destroys the bencoded data passed to it.
 * 
 * @retrun NULL on failure. On success, returns a pointer to the next byte following the parsed out value.
 */
char * bencode_parse(char *bencode_data, size_t length, size_t *length_remaining, bencoded_value **bencoded_value);

/**
 * 
 * 
 * TODO: There is a problem on 32-bit where integers are limited to INT_MAX, but the bencode specification does not place a limit on the size of an integer...
 * 
 * 
 * bencode_data			The bencoded data bytes to be parsed.
 * length				The length of bencode_data in bytes.
 * length_remaining		Out param. The number of bytes remaining in bencode_data after parsing out the integer.
 * byte_str				Out param. The parsed integer.
 * 
 * Note: This fucntion destroys the bencoded data passed to it.
 * 
 * @retrun NULL on failure. On success, returns a pointer to the next byte following the integer.
 */
char * bencode_parse_integer(char *bencode_data, size_t length, size_t *length_remaining, bencoded_int *integer);

/**
 * 
 * bencode_data			The bencoded data bytes to be parsed.
 * length				The length of bencode_data in bytes.
 * length_remaining		Out param. The number of bytes remaining in bencode_data after parsing out the byte string.
 * byte_str				Out param. The parsed byte string.
 * 
 * Note: This fucntion destroys the bencoded data passed to it.
 * 
 * @retrun NULL on failure. On success, returns a pointer to the next byte following the byte string.
 */
char * bencode_parse_byte_string(char *bencode_data, size_t length, size_t *length_remaining, bencoded_byte_str *byte_str);

/**
 * 
 * 
 * @retrun NULL on failure. On success, returns a pointer to the next byte following the list.
 */
char * bencode_parse_list(char *bencode_data, size_t length, size_t *length_remaining, bencoded_list *_list);

/**
 * 
 * 
 * @retrun NULL on failure. On success, returns a pointer to the next byte following the dictionary.
 */
char * bencode_parse_dict(char *bencode_data, size_t length, size_t *length_remaining, bencoded_dict *_dict);


}


#endif