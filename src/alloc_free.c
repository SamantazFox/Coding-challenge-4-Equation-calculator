#include "alloc_free.h"


/*
 * Allocator code: malloc memory and initializes the structures
*/

function_t* new_function_t(const function_def_t* params)
{
	// Allocate memory
	function_t* fun = malloc(
		sizeof(function_t) + params->argc * sizeof(equation_t*)
	);

	// Initialize function definition
	fun->definition = params;

	// Initialize parameter pointers
	for (int i = 0; i < params->argc; i++)
		fun->arguments[i] = (equation_t*) NULL;

	// Return pointer to the caller
	return fun;
}

equation_t* new_equation_t(void)
{
	// Allocate memory
	equation_t* eq = malloc (sizeof(equation_t));

	// Init operand A
	eq->elemA.type     = operand_type__NONE;
	eq->elemA.subtype  = operand_subtype__NAN;
	eq->elemA.uValue   = (uint64_t)    0ull;
	eq->elemA.function = (function_t*) NULL;
	eq->elemA.nestedEq = (equation_t*) NULL;

	// Init operand B
	eq->elemB.type     = operand_type__NONE;
	eq->elemB.subtype  = operand_subtype__NAN;
	eq->elemB.uValue   = (uint64_t)    0ull;
	eq->elemB.function = (function_t*) NULL;
	eq->elemB.nestedEq = (equation_t*) NULL;

	// Init equation type
	eq->operation = operation_type__NONE;

	// Return pointer to the caller
	return eq;
}



/*
 * Freeing code: frees the specified structure, recursively
*/

void free_function_t(function_t* fun)
{
	// Free any parameters of this function
	for (int i = 0; i < fun->definition->argc; i++)
	{
		if (fun->arguments[i]) free_equation_t(fun->arguments[i]);
	}

	// Free the memory containing this structure
	free(fun);
}

void free_equation_t(equation_t* eq)
{
	// If element A references nested eq/fun, free them here
	if (eq->elemA.function) free_function_t(eq->elemA.function);
	if (eq->elemA.nestedEq) free_equation_t(eq->elemA.nestedEq);

	// If element B references nested eq/fun, free them here
	if (eq->elemB.function) free_function_t(eq->elemB.function);
	if (eq->elemB.nestedEq) free_equation_t(eq->elemB.nestedEq);

	// Free the memory containing this structure
	free(eq);
}
