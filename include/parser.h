#ifndef __PARSER_H__
#define __PARSER_H__


#include <stdlib.h>
#include <string.h>

#include "print.h"

#include "structures.h"
#include "alloc_free.h"
#include "string_utils.h"


/*
 * Forward declarations
*/

extern equation_t* parseEquation(char* stringToParse, size_t stringLen);
extern function_t* parseFunction(char* stringToParse, size_t stringLen);


#endif  /* !__PARSER_H__ */
