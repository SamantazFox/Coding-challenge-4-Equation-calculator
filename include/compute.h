#ifndef __COMPUTE_H__
#define __COMPUTE_H__


#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>

#include "print.h"

#include "structures.h"


/*
 * Forward declarations
*/

extern double compute_function(function_t* fun);
extern double resolve_equation(equation_t* eq);


#endif  /* !__COMPUTE_H__ */
