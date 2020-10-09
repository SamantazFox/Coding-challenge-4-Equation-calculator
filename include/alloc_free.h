#ifndef __ALLOC_FREE_H__
#define __ALLOC_FREE_H__


#include <stdlib.h>

#include "structures.h"


/*
 * Forward declaration, because the function below use each other
*/

extern function_t* new_function_t(const function_def_t* params);
extern equation_t* new_equation_t(void);
extern void free_function_t(function_t* fun);
extern void free_equation_t(equation_t* eq);


#endif  /* !__ALLOC_FREE_H__ */
