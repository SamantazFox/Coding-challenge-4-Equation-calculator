#ifndef __PRINT_H__
#define __PRINT_H__


#include <stdio.h>


#define TRACE(_format, ...) \
	do { \
		fprintf(stdout, "[Trace] %s:%d\t" _format, __FILE__, __LINE__, __VA_ARGS__); \
	} while (0)

#define DEBUG(_format, ...) \
	do { \
		fprintf(stdout, "[Debug] %s:%d\t" _format, __FILE__, __LINE__, __VA_ARGS__); \
	} while (0)

#define INFO(_format, ...) \
	do { \
		fprintf(stdout, "[Info ] %s:%d\t" _format, __FILE__, __LINE__, __VA_ARGS__); \
	} while (0)

#define WARN(_format, ...) \
	do { \
		fprintf(stderr, "[Warn ] %s:%d\t" _format, __FILE__, __LINE__, __VA_ARGS__); \
	} while (0)

#define ERROR(_format, ...) \
	do { \
		fprintf(stderr, "[Error] %s:%d\t" _format, __FILE__, __LINE__, __VA_ARGS__); \
	} while (0)


#endif  /* !__PRINT_H__ */
