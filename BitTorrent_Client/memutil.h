/*
 * 
 * 
 * Note: The efficiency of these functions has not been tested, and 
 * they are likely sub-optimal.
 * 
 * It is necessary to define my own memmem() since the memmem() function 
 * is not present on Windows.
 * 
 * @author Daniel Liscinsky
 */



#ifndef MEM_UTIL_H
#define MEM_UTIL_H



#if defined(__cplusplus)
extern "C" {
#endif



/**
 Scans memory for the first occurrence of the given sequence of bytes.

 @param src				the memory to scan
 @param src_size		the size in bytes of the memory to scan
 @param pattern			the sequence of bytes to search for
 @param pattern_size	the size in bytes of the pattern sequence

 @return A pointer to the first occurrence of pattern in src, or NULL if pattern is not present in src.
 */
#if defined(_WIN32)
void * memmem(void *src, size_t src_size, const void *pattern, size_t pattern_size);
#endif


/**
 Scans memory for the first occurrence of the given byte.

 @param src				the memory to scan
 @param src_size		the size in bytes of the memory to scan
 @param _byte			the byte to search for

 @return A pointer to the first occurrence of _byte in src, or NULL if _byte is not present in src.
 */
//void * memmemb(void *src, size_t src_size, unsigned char _byte);



#if defined(__cplusplus)
}
#endif

#endif