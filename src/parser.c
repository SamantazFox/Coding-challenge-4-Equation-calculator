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

	TRACE("Looking for '%c'\n", item);

	for (int i = 0; i < len; i++)
		if (buf[i] == item) { pos = i; break; }

	// In the case where we found something, make sure that it is not nested
	// in a set of parenthesis. If this is the case, search again, starting
	// from that range's end, plus one (i.e: the character right after).
	if (subRange.exists && pos >= subRange.start && pos <= subRange.stop)
	{
		TRACE("Subrange from %d to %d, skipping\n", subRange.start, subRange.stop);
		pos = -1;

		// We're at the end of string, no need to continue!!
		if (subRange.stop == len-1) return pos;

		size_t newStart = subRange.stop + 1;
		size_t pos2 = strnfind(buf + newStart, len - newStart, item);

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
	// Search for an equation betwen parenthesis
	char*   start    = string + range->start;
	size_t  length   = range->stop - range->start + 1;
	range_t subrange = searchParenthesis(start, length);

	// If such equation exists and is the same length as the string, we
	// have an extra set of parenthesis to remove
	if (subrange.exists && subrange.start == 0 && subrange.stop == length - 1)
	{
		TRACE("Removing one (useless) pair of parenthesis!%c", '\n');

		range->start++;
		range->stop--;
		stripSurroundingParenthesis(string, range);
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
	int  idx_add, idx_sub, idx_mul, idx_div, idx_mod;
	bool has_add, has_sub, has_mul, has_div, has_mod;

	idx_add = strnfind(buffer, stringLen, '+'); has_add = (bool) (idx_add >= 0);
	idx_sub = strnfind(buffer, stringLen, '-'); has_sub = (bool) (idx_sub >= 0);
	idx_mul = strnfind(buffer, stringLen, '*'); has_mul = (bool) (idx_mul >= 0);
	idx_div = strnfind(buffer, stringLen, '/'); has_div = (bool) (idx_div >= 0);
	idx_mod = strnfind(buffer, stringLen, '%'); has_mod = (bool) (idx_mod >= 0);

	bool has_any = (has_add || has_sub || has_mul || has_div || has_mod);

	if (has_add) TRACE("Found '+' at %d\n", idx_add);
	if (has_sub) TRACE("Found '-' at %d\n", idx_sub);
	if (has_mul) TRACE("Found '*' at %d\n", idx_mul);
	if (has_div) TRACE("Found '/' at %d\n", idx_div);
	if (has_mod) TRACE("Found '%%' at %d\n", idx_mod);


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

			// The subrange is maybe a 'n [times] (y)' type of equation,
			// where the multiplication is implied.
			else if (subEqRange.stop == stringLen - 1)
			{
				// Compute ranges
				range_t r1 = functionRange;
				range_t r2 = subEqRange;

				DEBUG("Equation: %s\n", buffer);
				TRACE("Range 1: start = %d / stop = %d\n", r1.start, r1.stop);
				TRACE("Range 2: start = %d / stop = %d\n", r2.start, r2.stop);

				// Parse range 1 as a number
				ret->elemA = parseNumberRange(buffer, r1);

				if (ret->elemA.subtype == operand_subtype__NAN)
					goto InvalidInputError;

				// Parse range 2 as a sub-equation
				ret->elemB.nestedEq = parseEquation(buffer, r2);
				ret->elemB.type = operand_type__NESTED_EQ;

				if (ret->elemB.nestedEq == NULL)
					goto InvalidInputError;

				// Set operation type and exit
				ret->operation = operation_type__TIMES;
				goto ExitNominal;
			}

			// Same as above, but inverted (i.e '(y) [times] n')
			// This is not conventional, but let's support it
			else if (subEqRange.start == 0)
			{
				// Compute ranges
				range_t r1 = subEqRange;
				range_t r2 = { .start = subEqRange.stop+1, .stop = stringLen-1 };

				DEBUG("Equation: %s\n", buffer);
				TRACE("Range 1: start = %d / stop = %d\n", r1.start, r1.stop);
				TRACE("Range 2: start = %d / stop = %d\n", r2.start, r2.stop);

				// Parse range 1 as a sub-equation
				ret->elemA.nestedEq = parseEquation(buffer, r1);
				ret->elemA.type = operand_type__NESTED_EQ;

				if (ret->elemA.nestedEq == NULL)
					goto InvalidInputError;

				// Parse range 2 as a number
				ret->elemB = parseNumberRange(buffer, r2);

				if (ret->elemB.subtype == operand_subtype__NAN)
					goto InvalidInputError;

				// Set operation type and exit
				ret->operation = operation_type__TIMES;
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

	// Fourth case: modulo operation
	else if (has_mod)
	{
		DEBUG("Equation: %s\n", buffer);

		// Compute ranges
		range_t r1 = { .start = 0, .stop = idx_mod - 1 };
		range_t r2 = { .start = idx_mod + 1, .stop = stringLen - 1 };

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

		ret->operation = operation_type__MOD;
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
		ERROR("Invalid length (0) when parsing function%c", '\n');
		return NULL;
	}

	// copy the sub-equation string to new buffer
	char* buffer = strsafecpy(stringToParse + rangeToParse.start, stringLen);

	DEBUG("Function: '%s'\n", buffer);


	// Look up the arguments
	range_t argumentsRange = searchParenthesis(buffer, stringLen);


	if (argumentsRange.exists)
	{
		TRACE("Raw arguments: '%.*s'\n",
			argumentsRange.stop - argumentsRange.start +1,
			buffer + argumentsRange.start
		);
		stripSurroundingParenthesis(buffer, &argumentsRange);

		// These checks must be done AFTER removeing the parenthesis
		if (argumentsRange.start > argumentsRange.stop)
		{
			// Check for arguments emptyness '()'
			ERROR("Invalid function '%s': no arguments found\n", buffer);
			goto FormatError;
		}
		else if (argumentsRange.stop != stringLen-2)
		{
			// Check for arguments malformation '(...)garbage'
			ERROR("Invalid function '%s': Malformed expression\n", buffer);
			goto FormatError;
		}
	}
	else
	{
		ERROR("Invalid function '%s': no arguments found\n", buffer);
		goto FormatError;
	}


	// Verify that this corresponds to a known function
	range_t funcNameRange = { .start = 0, .stop = argumentsRange.start - 2 };
	const function_def_t* definition = validateFunNameRange(buffer, funcNameRange);

	if (definition == NULL)
	{
		ERROR("Unsupported function '%.*s'\n", funcNameRange.stop+1, buffer);
		goto FormatError;
	}

	DEBUG("Function: %s(), Expecting %d arguments\n",
		definition->name, definition->argc
	);


	// Init the returned pointer with an empty structure
	function_t* ret = new_function_t(definition);


	// Count and separate function's arguments
	int positionCounter = argumentsRange.start;
	int previousCommaPos = -1;

	for (int i = 0; i < definition->argc; i++)
	{
		// Search for comma(s) while avoiding sub-equations
		int countToEnd = argumentsRange.stop - positionCounter;
		int commaPos = strnfind(buffer + positionCounter, countToEnd, ',');

		// Error handling
		if (commaPos == (previousCommaPos+1))
		{
			ERROR("Empty argument detected in equation '%s'\n", buffer);
			goto ArgumentError;
		}
		previousCommaPos = commaPos;

		// Save argument's details
		range_t argumentRange = {0};

		if (commaPos != -1)
		{
			// Increment position counter to arrive at comma index,
			// save this index - 1 as the end of argN,
			// then increment position counter again to skip comma
			argumentRange.start = positionCounter;
			positionCounter += commaPos;

			argumentRange.stop = positionCounter - 1;
			positionCounter++;

			TRACE("\tArg #%d: %.*s\n", i + 1,
				(int) argumentRange.stop - argumentRange.start + 1,
				buffer + argumentRange.start
			);

			ret->arguments[i] = parseEquation(buffer, argumentRange);
		}
		else  // (commaPos == -1)
		{
			if (i == (definition->argc - 1))
			{
				argumentRange.start = positionCounter;
				argumentRange.stop  = argumentsRange.stop;

				TRACE("\tArg #%d: %.*s\n", i + 1,
					(int) argumentRange.stop - argumentRange.start + 1,
					buffer + argumentRange.start
				);

				ret->arguments[i] = parseEquation(buffer, argumentRange);
				break;
			}
			else
			{
				ERROR("Function: %s(), Found %d args, expected %d\n",
					definition->name, i, definition->argc
				);
				goto ArgumentError;
			}
		}

	}


	// Cleanup needed stuff, then return the structure pointer
	if (buffer) free(buffer);
	return ret;

FormatError:
	// Ran when 'ret' is not set yet
	if (buffer) free(buffer);
	return NULL;

ArgumentError:
	// Ran once 'ret' has been allocated
	if (buffer) free(buffer);
	free_function_t(ret);
	return NULL;
}
