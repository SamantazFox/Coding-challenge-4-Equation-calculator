#include "compute.h"


/*
 * Function declarations
*/

double compute_function(function_t* fun)
{
	// define a macro to compare the function name with 'str'
	#define compareName(str) (strncmp(fun->name, str, sizeof(str)) == 0)

	// Make sure that the STD version is high enough to support all the
	// function listed below
	assert(__STDC_VERSION__ >= 201112L);

	/*
	 * Functions with 1 argument
	*/

	assert(fun->argCount >= 1);
	assert(fun->arguments[0] != NULL);

	double x = resolve_equation(fun->arguments[0]);

	// Trigonometric functions
	if (compareName("cos"  )) return cos  (x);
	if (compareName("sin"  )) return sin  (x);
	if (compareName("tan"  )) return tan  (x);
	if (compareName("acos" )) return acos (x);
	if (compareName("asin" )) return asin (x);
	if (compareName("atan" )) return atan (x);

	// Hyperbolic functions
	if (compareName("cosh" )) return cosh (x);
	if (compareName("sinh" )) return sinh (x);
	if (compareName("tanh" )) return tanh (x);
	if (compareName("acosh")) return acosh(x);
	if (compareName("asinh")) return asinh(x);
	if (compareName("atanh")) return atanh(x);

	// Exponential and logarithmic functions
	if (compareName("exp"  )) return exp  (x);
	if (compareName("log"  )) return log  (x);
	if (compareName("log10")) return log10(x);
	if (compareName("exp2" )) return exp2 (x);
	if (compareName("expm1")) return expm1(x);
	if (compareName("log1p")) return log1p(x);
	if (compareName("log2" )) return log2 (x);
	if (compareName("logb" )) return logb (x);

	// Power functions
	if (compareName("sqrt")) return sqrt(x);
	if (compareName("cbrt")) return cbrt(x);

	// Rounding and remainder functions
	if (compareName("ceil" )) return ceil (x);
	if (compareName("floor")) return floor(x);
	if (compareName("trunc")) return trunc(x);
	if (compareName("round")) return round(x);
	if (compareName("rint" )) return rint (x);

	// Error and gamma functions
	if (compareName("erf" )) return erf (x);
	if (compareName("erfc")) return erfc(x);
	if (compareName("tgamma")) return tgamma(x);
	if (compareName("lgamma")) return lgamma(x);

	// Other functions
	if (compareName("abs")) return fabs(x);


	/*
	 * Functions with 2 arguments
	*/

	assert(fun->argCount >= 2);
	assert(fun->arguments[1] != NULL);

	double y = resolve_equation(fun->arguments[1]);

	// Trigonometric functions
	if (compareName("atan2")) return atan2(x, y);

	// Power functions
	if (compareName("pow"  )) return pow  (x, y);
	if (compareName("hypot")) return hypot(x, y);

	// Rounding and remainder functions
	if (compareName("fmod" )) return fmod(x, y);

	// Other functions
	if (compareName("min" )) return fmin(x, y);
	if (compareName("max" )) return fmax(x, y);
	if (compareName("fdim")) return fdim(x, y);


	/*
	 * Default target: display a warning and return 0
	*/

	WARN("Invalid function '%s'.", fun->name);
	return 0.0;
}


double resolve_equation(equation_t* eq)
{
	// If A is a nested equation or a function, and hasn't been solved, do it here
	if (eq->elemA.subtype == operand_subtype__NAN)
	{
		if (eq->elemA.type == operand_type__NESTED_EQ)
		{
			assert(eq->elemA.nestedEq != NULL);
			eq->elemA.dValue  = resolve_equation(eq->elemA.nestedEq);
			eq->elemA.subtype = operand_subtype__DOUBLE;
		}
		else if (eq->elemA.type == operand_type__FUNCTION)
		{
			assert(eq->elemA.function != NULL);
			eq->elemA.dValue  = compute_function(eq->elemA.function);
			eq->elemA.subtype = operand_subtype__DOUBLE;
		}
		else
		{
			ERROR("Invalid operand type '%d' provided for 'A'.\n", eq->elemA.type);
			exit(1);
		}
	}

	// Check that A has a valid subtype and convert it to a double
	double a;
	switch (eq->elemA.subtype)
	{
		case operand_subtype__INT:    a = (double) eq->elemA.iValue; break;
		case operand_subtype__UINT:	  a = (double) eq->elemA.uValue; break;
		case operand_subtype__DOUBLE: a = (double) eq->elemA.dValue; break;
		default:
			ERROR("Invalid operand subtype '%d' provided for 'A'.\n", eq->elemA.subtype);
			exit(1);
	}


	// If there is no B operand, return the numeric value of A
	if (eq->elemB.type == operand_type__NONE || eq->operation == operation_type__NONE)
		return a;


	// If B is a nested equation or a function, and hasn't been solved, do it here
	if (eq->elemB.subtype == operand_subtype__NAN)
	{
		if (eq->elemB.type == operand_type__NESTED_EQ)
		{
			assert(eq->elemB.nestedEq != NULL);
			eq->elemB.dValue  = resolve_equation(eq->elemB.nestedEq);
			eq->elemB.subtype = operand_subtype__DOUBLE;
		}
		else if (eq->elemB.type == operand_type__FUNCTION)
		{
			assert(eq->elemB.function != NULL);
			eq->elemB.dValue  = compute_function(eq->elemB.function);
			eq->elemB.subtype = operand_subtype__DOUBLE;
		}
		else
		{
			ERROR("Invalid operand type '%d' provided for 'A'.\n", eq->elemB.type);
			exit(1);
		}
	}

	// Check that B has a valid subtype and convert it to a double
	double b;
	switch (eq->elemB.subtype)
	{
		case operand_subtype__INT:    b = (double) eq->elemB.iValue; break;
		case operand_subtype__UINT:	  b = (double) eq->elemB.uValue; break;
		case operand_subtype__DOUBLE: b = (double) eq->elemB.dValue; break;
		default:
			ERROR("Invalid operand subtype '%d' provided for 'B'.\n", eq->elemB.subtype);
			exit(1);
	}


	// Compute the final result
	switch (eq->operation)
	{
		case operation_type__PLUS:  return (a + b);
		case operation_type__MINUS: return (a - b);
		case operation_type__TIMES: return (a * b);
		case operation_type__DIV:   return (b == 0) ? 0 : (a / b);
		case operation_type__MOD:   return (b == 0) ? a : fmod(a, b);
		case operation_type__POW:   return pow(a, b);
		default:
			ERROR("Invalid operation type '%d' provided.\n", eq->operation);
			exit(1);
	}
}
