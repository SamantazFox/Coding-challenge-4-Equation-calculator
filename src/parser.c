#include "parser.h"

#include "charcmp.h"
#include "functions.h"

#include "assert.h"


#define min(a,b) ((a<b) ? a : b)
#define max(a,b) ((a>b) ? a : b)


/************************************************\
 *
 * String find / search utils
 *
\************************************************/

range_t searchParenthesis(char* buf, size_t len)
{
	int parenthesisCount = 0;
	range_t subEq = {
		.exists = false,
		.start = 0,
		.stop = 0
	};

	for (int i = 0; i < len; i++)
	{
		if (buf[i] == '(')
		{
			parenthesisCount++;

			// First parenthesis encountered,
			// activate trigger and save starting position
			if (parenthesisCount == 1 && !subEq.exists)
			{
				subEq.exists = true;
				subEq.start = i;
			}
		}

		if (buf[i] == ')')
		{
			parenthesisCount--;

			// Final (closing) parenthesis encountered,
			// save end position and exit
			if (parenthesisCount == 0 && subEq.exists)
			{
				subEq.stop = i;
				break;
			}
		}
	}

	return subEq;
}

size_t strnfind(char* buf, int len, const char item)
{
	size_t pos = -1;
	range_t subRange = searchParenthesis(buf, len);

	for (int i = 0; i < len; i++)
	{
		TRACE("Looking for '%c' at relative index % 3d ('%c')\n", item, i, buf[i]);
		if (buf[i] == item) { pos = i; break; }
	}

	// In the case where we found something, make sure that it is not nested
	// in a set of parenthesis. If this is the case, search again, starting
	// from that range's end, plus one (i.e: the character right after).
	if (subRange.exists && pos >= subRange.start && pos <= subRange.stop)
	{
		TRACE("Subrange from %d to %d, skipping\n", subRange.start, subRange.stop);
		pos = -1;

		size_t newStart = subRange.stop + 1;
		size_t pos2 = strnfind(buf + newStart, len - 1 - newStart, item);

		// pos2 is relative to newStart, so don't forget to add it back here
		if (pos2 != -1) pos = newStart + pos2;
	}

	return pos;
}



/************************************************\
 *
 * String edit utils
 *
\************************************************/

static void stripSurroundingParenthesis(char* string, range_t* range)
{
	for (int i = range->start, j = range->stop; i < j; i++, j--)
	{
		if (string[i] == '(' && string[j] == ')')
		{
			TRACE("Removing one (useless) pair of parenthesis!%c", '\n');

			range->start++;
			range->stop--;
		}
		else break;
	}
}



/************************************************\
 *
 * Parsing utils
 *
\************************************************/

static operand_t parseNumber(char* buffer, size_t stringLen)
{
	bool isFloat = false;
	bool isValid = true;

	operand_t number = {
		.type    = operand_type__CONST,
		.subtype = operand_subtype__NAN,
		.iValue  = 0
	};

	for (int i = 0; i < stringLen; i++)
	{
		char c = buffer[i];

		// If charater is not a digit or a dot '.', invalid number
		if (!isNumber(c)) { isValid = false; break; }

		// Check if the number contains a dot (== double)
		if (c == '.')
		{
			// If a dot was already detected: invalid number
			// Otherwise turn on flag and continue
			if (isFloat) { isValid = false; break; }
			else isFloat = true;
		}
	}

	if (isValid)
	{
		// Number is a double
		if (isFloat)
		{
			INFO("Found an double !! '%.*s'\n", (int) stringLen, buffer);
			number.subtype = operand_subtype__DOUBLE;
			number.dValue = atof( (const char*) buffer );
		}

		// Number is an integer
		else
		{
			INFO("Found an integer !! '%.*s'\n", (int) stringLen, buffer);
			number.subtype = operand_subtype__INT;
			number.iValue = atol( (const char*) buffer );
		}
	}

	return number;
}

static operand_t parseNumberRange(char* buffer, range_t range)
{
	size_t length = range.stop - range.start + 1;
	return parseNumber(buffer + range.start, length);
}


static const function_def_t* validateFunName(char* buffer, size_t stringLen)
{
	// Extract substring from buffer, so strcmp can work with it
	char localbuffer[stringLen+1];
	strsafecpy2(localbuffer, buffer, stringLen);

	// try to find this function's name in the supported list
	for (int i = 0; i < known_functions_length_g; i++)
	{
		const function_def_t* temp = &known_functions_g[i];
		if (strcmp(localbuffer, temp->name) == 0) return temp;
	}

	// Nothing was found
	return NULL;
}

static const function_def_t* validateFunNameRange(char* buffer, range_t range)
{
	size_t length = range.stop - range.start + 1;
	return validateFunName(buffer + range.start, length);
}



/************************************************\
 *
 * Core parsing functions - Parse equation
 *
\************************************************/

equation_t* parseEquation(char* stringToParse, range_t rangeToParse)
{
	// Remove extra parenthesis (e.g: '((x))' => 'x' ) around equation,
	// then compute string length from range given as arg #2
	// (we add one because we want to include both start and stop)
	if (rangeToParse.start > rangeToParse.stop)
	{
		ERROR("Invalid range: start = %u / stop = %u\n",
			rangeToParse.start,
			rangeToParse.stop
		);
		return NULL;
	}

	stripSurroundingParenthesis(stringToParse, &rangeToParse);
	size_t stringLen = (size_t) (rangeToParse.stop - rangeToParse.start + 1);

	if (stringLen == 0)
	{
		ERROR("Invalid length (0) when parsing equation '%s'\n", stringToParse);
		return NULL;
	}

	// copy the sub-equation string to new buffer
	char* buffer = strsafecpy(stringToParse + rangeToParse.start, stringLen);

	// Init the returned pointer with an empty structure
	equation_t* ret = new_equation_t();


	// Determine what is the closest element between '+', '-', '*' and '/'
	// while taking care to avoid nested equations
	int  idx_add, idx_sub, idx_mul, idx_div;
	bool has_add, has_sub, has_mul, has_div;

	idx_add = strnfind(buffer, stringLen, '+'); has_add = (bool) (idx_add >= 0);
	idx_sub = strnfind(buffer, stringLen, '-'); has_sub = (bool) (idx_sub >= 0);
	idx_mul = strnfind(buffer, stringLen, '*'); has_mul = (bool) (idx_mul >= 0);
	idx_div = strnfind(buffer, stringLen, '/'); has_div = (bool) (idx_div >= 0);

	bool has_any = (has_add || has_sub || has_mul || has_div);

	if (has_add) TRACE("Found '+' at %d\n", idx_add);
	if (has_sub) TRACE("Found '-' at %d\n", idx_sub);
	if (has_mul) TRACE("Found '*' at %d\n", idx_mul);
	if (has_div) TRACE("Found '/' at %d\n", idx_div);


	// Hardest case: Nothing was found
	if (!has_any)
	{
		// Search for a series of parenthesis
		range_t subEqRange = searchParenthesis(buffer, stringLen);

		if (subEqRange.exists)
		{
			// Try to parse that as a function
			range_t functionRange = { .start = 0, .stop = stringLen - 1 };
			function_t* tryParseFunction = parseFunction(buffer, functionRange);

			// Valid function
			if (tryParseFunction != NULL)
			{
				ret->elemA.type = operand_type__FUNCTION;
				ret->elemA.function = tryParseFunction;
				goto ExitNominal;
			}

			// Unable to parse input
			goto InvalidInputError;
		}


		// Try to parse as a number if possible
		operand_t tryParseNumber = parseNumber(buffer, stringLen);

		if (tryParseNumber.subtype != operand_subtype__NAN)
		{
			ret->elemA = tryParseNumber;
			goto ExitNominal;
		}

		// Input can't be parsed
		goto InvalidInputError;
	}

	// Second case: there is an addition or a substraction
	else if (has_add || has_sub)
	{
		// Short form to get the index of the first item ('-' or '+')
		// even if one of them is -1
		int add_sub = (has_add && has_sub)
			? min(idx_add, idx_sub)
			: max(idx_add, idx_sub);

		DEBUG("Equation: %s\n", buffer);

		// Compute ranges
		range_t r1 = { .start = 0, .stop = add_sub - 1 };
		range_t r2 = { .start = add_sub + 1, .stop = stringLen - 1 };

		TRACE("Range 1: start = %d / stop = %d\n", r1.start, r1.stop);
		TRACE("Range 2: start = %d / stop = %d\n", r2.start, r2.stop);

		// Parse sub-equations
		equation_t* eq1 = parseEquation(buffer, r1);
		if (eq1 == NULL) goto SubFunctionError;

		equation_t* eq2 = parseEquation(buffer, r2);
		if (eq2 == NULL) { free_equation_t(eq1); goto SubFunctionError; }

		// Fill return structure
		ret->elemA.nestedEq = eq1; ret->elemA.type = operand_type__NESTED_EQ;
		ret->elemB.nestedEq = eq2; ret->elemB.type = operand_type__NESTED_EQ;

		if(idx_add == add_sub) ret->operation = operation_type__PLUS;
		if(idx_sub == add_sub) ret->operation = operation_type__MINUS;
	}

	// Third case: there is a multiplication or a division
	else if (has_mul || has_div)
	{
		// Short form to get the index of the first item ('*' or '/')
		// even if one of them is -1
		int mul_div = (has_mul && has_div)
			? min(idx_mul, idx_div)
			: max(idx_mul, idx_div);

		DEBUG("Equation: %s\n", buffer);

		// Compute ranges
		range_t r1 = { .start = 0, .stop = mul_div - 1 };
		range_t r2 = { .start = mul_div + 1, .stop = stringLen - 1 };

		TRACE("Range 1: start = %d / stop = %d\n", r1.start, r1.stop);
		TRACE("Range 2: start = %d / stop = %d\n", r2.start, r2.stop);

		// Parse sub-equations
		equation_t* eq1 = parseEquation(buffer, r1);
		if (eq1 == NULL) goto SubFunctionError;

		equation_t* eq2 = parseEquation(buffer, r2);
		if (eq2 == NULL) { free_equation_t(eq1); goto SubFunctionError; }

		// Fill return structure
		ret->elemA.nestedEq = eq1; ret->elemA.type = operand_type__NESTED_EQ;
		ret->elemB.nestedEq = eq2; ret->elemB.type = operand_type__NESTED_EQ;

		if(idx_mul == mul_div) ret->operation = operation_type__TIMES;
		if(idx_div == mul_div) ret->operation = operation_type__DIV;
	}


ExitNominal:
	return ret;


InvalidInputError:
	// Input can't be parsed
	ERROR("Invalid input !! '%s'\n", buffer);

SubFunctionError:
	free_equation_t(ret);
	return NULL;
}



/************************************************\
 *
 * Core parsing functions - Parse function
 *
\************************************************/

function_t* parseFunction(char* stringToParse, range_t rangeToParse)
{
	// Remove extra parenthesis (e.g: '((x))' => 'x' ) around equation,
	// then compute string length from range given as arg #2
	// (we add one because we want to include both start and stop)
	if (rangeToParse.start > rangeToParse.stop)
	{
		ERROR("Invalid range: start = %u / stop = %u",
			rangeToParse.start,
			rangeToParse.stop
		);
		return NULL;
	}

	stripSurroundingParenthesis(stringToParse, &rangeToParse);
	size_t stringLen = (size_t) (rangeToParse.stop - rangeToParse.start + 1);

	if (stringLen == 0)
	{
		ERROR("Invalid length (0) when parsing function '%s'", stringToParse);
		return NULL;
	}

	// Copy sub-equation string to new buffer
	// Then replace surrounding parenthesis with '\0'
	char* buffer = strsafecpy(stringToParse + rangeToParse.start, stringLen);

	DEBUG("Function: '%s'\n", buffer);


	// Copy function name to it's own buffer
	// We also have to keep track of arguments start/stop points
	char functionName[FUNCTION_NAME_MAXLEN];
	functionName[FUNCTION_NAME_MAXLEN-1] = '\0';

	range_t argumentsRange = { .start = 0, .stop = stringLen - 1 };

	for (int i = 0; i < (FUNCTION_NAME_MAXLEN-1); i++)
	{
		char c = buffer[i];

		if ( isalnum(c) )
		{
			functionName[i] = c;
		}
		else if (c == '(')
		{
			functionName[i] = '\0';

			// Arguments are starting here, so mark it and use
			// stripSurroundingParenthesis() to isolate the arguments
			argumentsRange.start = i;
			stripSurroundingParenthesis(buffer, &argumentsRange);
			break;
		}
		else
		{
			ERROR("Invalid function '%s'", buffer);
			free(buffer);
			return NULL;
		}
	}


	// Count and separate function's arguments
	int argsCounter = 0;
	int positionCounter = argumentsRange.start;
	int previousCommaPos = -1;

	range_t argRanges[5] = { 0 };

	do
	{
		// Search for comma(s) while avoiding sub-equations
		int countToEnd = argumentsRange.stop - positionCounter;
		int commaPos = strnfind(buffer + positionCounter, countToEnd, ',');

		// Error handling
		if (commaPos == (previousCommaPos+1))
		{
			ERROR("Empty argument detected in equation '%s'\n", buffer);
			free(buffer);
			return NULL;
		}
		previousCommaPos = commaPos;

		// Save argement's details
		argRanges[argsCounter].start = positionCounter;

		if (commaPos != -1)
		{
			// Increment position counter to arrive at comma index,
			// save this index - 1 as the end of argN,
			// then increment position counter again to skip comma
			positionCounter += commaPos;
			argRanges[argsCounter++].stop = positionCounter - 1;
			positionCounter++;
		}
		else
		{
			argRanges[argsCounter++].stop = argumentsRange.stop;
			break;
		}

	}
	while (argsCounter < 5);


	TRACE("Function: %s(), Found %d args\n", functionName, argsCounter);


	// Init the returned pointer with an empty structure
	// Then, fill the structure
	function_t* ret = new_function_t(argsCounter);
	strncpy(ret->name, functionName, FUNCTION_NAME_MAXLEN);

	for (int i = 0; i < argsCounter; i++)
		ret->arguments[i] = parseEquation(buffer, argRanges[i]);


	// Cleanup needed stuff, then return the structure pointer
	free(buffer);
	return ret;
}
