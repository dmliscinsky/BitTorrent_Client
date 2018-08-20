/*
 * @author Daniel Liscinsky
 */

#include <string.h>

#include "memutil.h"


#if defined(_WIN32)
void * memmem(void *src, size_t src_size, const void *pattern, size_t pattern_size) {

	while (src_size >= pattern_size && memcmp(src, pattern, pattern_size)) {
		src_size--;
		src = (char *)src + 1;
	}

	if (src_size < pattern_size) {
		return NULL;
	}
	
	return src;
}
#endif
/*
void * memmemb(void *src, size_t src_size, unsigned char _byte) {

	unsigned char *curr_ptr = (unsigned char *)src;
	unsigned char *end_ptr = (unsigned char *)src + src_size;

	while (curr_ptr < end_ptr && *curr_ptr != _byte) {
		curr_ptr++;
	}

	if (curr_ptr == end_ptr) {
		return NULL;
	}

	return curr_ptr;
}
*/