#ifndef __PARSER_H__
#define __PARSER_H__


#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "print.h"

#include "structures.h"
#include "alloc_free.h"
#include "string_utils.h"
#include "functions.h"


/*
 *
 * DATA STRUCURES
*/

typedef struct
{
	unsigned int start;
	unsigned int stop;
	bool exists;
}
range_t;


/*
 * Forward declarations
*/

extern equation_t* parseEquation(char* stringToParse, range_t rangeToParse);
extern function_t* parseFunction(char* stringToParse, range_t rangeToParse);


#endif  /* !__PARSER_H__ */
