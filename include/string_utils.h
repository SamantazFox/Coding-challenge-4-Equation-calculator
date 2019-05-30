#ifndef __STRING_UTILS_H__
#define __STRING_UTILS_H__


#include <string.h>


static inline char* strsafecpy(char* src, int len)
{
	// Init buffer
	char* dest = malloc( (len+1) * sizeof(char) );

	// Copy string to buffer and properly end string with 'nul' char
	strncpy(dest, src, len);
	dest[len] = '\0';

	// return result
	return dest;
}

static inline char* strsafecpy2(char* dest, char* src, int len)
{
	// Copy string to buffer and properly end string with 'nul' char
	strncpy(dest, src, len);
	dest[len] = '\0';

	// return result
	return dest;
}


static inline char* strnchrf(char* buf, int len, int c)
{
	// Search for the first occurence of 'c'
	for (int i = 0; i < len; i++) if (*(buf+i) == c) return (buf+i);
	return NULL;
}

static inline char* strnchrl(char* buf, int len, int c)
{
	// Search for the last occurence of 'c'
	for (int i = len; i >= 0; i--) if (*(buf+i) == c) return (buf+i);
	return NULL;
}


#endif  /* !__STRING_UTILS_H__ */
