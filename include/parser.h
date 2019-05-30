#ifndef __PARSER_H__
#define __PARSER_H__


#include <stdlib.h>
#include <string.h>

#include "print.h"

#include "structures.h"
#include "alloc_free.h"


/*
 * Forward declarations
*/

extern equation_t* parseEquation(char* start, size_t len);


#endif  /* !__PARSER_H__ */
