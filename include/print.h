#ifndef __PRINT_H__
#define __PRINT_H__


#include <stdio.h>


typedef enum
{
	level_NONE,
	level_ERROR,
	level_WARN,
	level_INFO,
	level_DEBUG,
	level_TRACE
}
loglvl_e;

extern loglvl_e loglevel;


#define TRACE(_format, ...) \
	if (loglevel >= level_TRACE) \
	do { \
		fprintf(stdout, "[Trace] %s:%d\t" _format, __FILE__, __LINE__, __VA_ARGS__); \
	} while (0)

#define DEBUG(_format, ...) \
	if (loglevel >= level_DEBUG) \
	do { \
		fprintf(stdout, "[Debug] %s:%d\t" _format, __FILE__, __LINE__, __VA_ARGS__); \
	} while (0)

#define INFO(_format, ...) \
	if (loglevel >= level_INFO) \
	do { \
		fprintf(stdout, "[Info ] %s:%d\t" _format, __FILE__, __LINE__, __VA_ARGS__); \
	} while (0)

#define WARN(_format, ...) \
	if (loglevel >= level_WARN) \
	do { \
		fprintf(stderr, "[Warn ] %s:%d\t" _format, __FILE__, __LINE__, __VA_ARGS__); \
	} while (0)

#define ERROR(_format, ...) \
	if (loglevel >= level_ERROR) \
	do { \
		fprintf(stderr, "[Error] %s:%d\t" _format, __FILE__, __LINE__, __VA_ARGS__); \
	} while (0)


#endif  /* !__PRINT_H__ */
