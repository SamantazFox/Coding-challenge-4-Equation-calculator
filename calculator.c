/*
 * Challenge
 *
 * Make a calculator that can calculate nested expressions, e.g. log(2x) + sin(5x/sqrt(pi)).
 *
 * mmmh, how to handle "x"es?
 *
 * It asks you to provide the value of x and any other unknown symbols it encounters.
 * Alternatively, it can find roots (such xs for which f x = 0).
 * Next level is plotting the graphs of those variables, like so:
*/


#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "print.h"

#include "structures.h"
#include "alloc_free.h"
#include "charcmp.h"

#include "parser.h"
#include "compute.h"


int main(int argc, char const *argv[])
{
	if (argc <= 1)
	{
		fprintf(stderr, "Usage: %s [equation]\n", argv[0]);
		return 0;
	}

	const char* equation = argv[1];
	size_t equationLen = strlen(equation);


	// Validate equation string while copying it to new buffer
	// Also strip spaces/tabs from the equation string
	char buffer[equationLen+1];
	buffer[equationLen] = '\0';
	int counter = 0;

	int nestedEquations    = 0;
	int closingParenthesis = 0;

	for (int i = 0; i < equationLen; i++)
	{
		char c = equation[i];

		if (isspace(c)) continue;
		if (!isValidChar(c))
		{
			ERROR(
				"The equation is malformed.\n"
				"Unknown char '%c' at position %d.\n", c, i
			);
			return 1;
		}

		if (c == '(') nestedEquations++;
		if (c == ')') closingParenthesis++;

		buffer[counter++] = c;
	}

	if ((nestedEquations - closingParenthesis) != 0)
	{
		ERROR("One or more sub-equations are missing parenthesis.%s\n", "\n");
		return 1;
	}

	// Print the new, verified and re-formatted string
	buffer[counter] = '\0';
	INFO("Input equation: %s\n", buffer);

	// Parse the equation
	range_t fullRange = { .start = 0, .stop = strlen(buffer) - 1 };
	equation_t* eq = parseEquation(buffer, fullRange);

	equation_t* eq = parseEquation(buffer, counter);
	double result = 0;

	if (eq != NULL)
		result  = resolve_equation(eq);

	INFO("Result: %f\n", result);
	return 0;
}
