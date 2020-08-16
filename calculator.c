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
loglvl_e loglevel = level_ERROR; // global

#include "structures.h"
#include "alloc_free.h"
#include "charcmp.h"

#include "parser.h"
#include "compute.h"


void printUsage(void)
{
	fprintf(stderr, "Usage: calc [options] '<equation>'\n\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -h    Displays this help mpessage\n");
	fprintf(stderr, "  -v    Increase verbosity (can be used multiple times)\n");
	fprintf(stderr, "  -q    Do not print errors\n");
}


int main(int argc, char const *argv[])
{
	const char* equation = NULL;

	// Make sure we have the required arguments
	if (argc < 1) { printUsage(); return 0; }

	// Parse argumants
	for (int i = (argc-1); i > 0; i--)
	{
		// Command
		if (strlen(argv[i]) == 2 && argv[i][0] == '-')
		{
			switch (argv[i][1])
			{
				case 'h': printUsage(); return 0;
				case 'v': if (loglevel != level_NONE) loglevel++; break;
				case 'q': loglevel = level_NONE; break;
				default:
					// Low probability, but can be an equation like '-1' or '-e'
					if (argc == 2 && equation == NULL)
						equation = argv[i];

					// Otherwise bad input
					else
					{
						fprintf(stderr, "Unknown option '%s'.\n\n", argv[i]);
						printUsage();
						return 2;
					}
			}
		}

		// Equation
		else if (equation == NULL) equation = argv[i];
	}

	// We weren't given any equation
	if (equation == NULL)
	{
		fprintf(stderr, "No equation given!\n\n");
		printUsage();
		return 3;
	}


	// Validate equation string while copying it to new buffer
	// Also strip spaces/tabs from the equation string
	size_t equationLen = strlen(equation);
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

	// Solve the equation
	if (eq == NULL) return 1;
	double result = resolve_equation(eq);

	// Print result to temporary buffer
	char display[60];
	snprintf(display, 60, "%f", result);

	// Remove any trailing zeroes
	uint8_t len = strlen(display);
	for (int i = (len-1); i > 0; i--)
	{
		if(display[i] == '0')
		{
			display[i] = '\0';
			if(display[i-1] == '.') { display[i-1] = '\0'; break; }
		}
		else break;
	}

	// Print result to user and exit
	printf("%s\n", display);
	return 0;
}
