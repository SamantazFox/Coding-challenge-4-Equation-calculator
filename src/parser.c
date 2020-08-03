#include "parser.h"

#include "charcmp.h"
#include "assert.h"


#define min(a,b) ((a<b) ? a : b)
#define max(a,b) ((a>b) ? a : b)


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


size_t strnfind(char* buf, int len, const char item)
{
	size_t pos = -1;
	range_t subRange = searchParenthesis(buf, len);

	for (int i = 0; i < len; i++) if (buf[i] == item) { pos = i; break; }

	if (subRange.exists && pos >= subRange.start && pos <= subRange.stop)
		return strnfind(buf + subRange.stop, len - 1 - subRange.stop, item);

	return pos;
}


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


	// Easiest case: Nothing was found
	if (!has_any)
	{
		// Is it a valid number ?
		bool containsDot = false;
		bool validNumber = true;

		for (int i = 0; i < stringLen; i++)
		{
			char c = buffer[i];

			// If charater is not a digit or a dot '.', invalid number
			if (!isNumber(c)) { validNumber = false; break; }

			// Check if the number contains a dot (== double)
			if (c == '.')
			{
				// If a dot was already detected: invalid number
				// Otherwise turn on flag and continue
				if (containsDot) { validNumber = false; break; }
				else validNumber = true;
			}
		}

		// Parse the number if possible
		if (validNumber)
		{
			INFO("Found an number !! '%s'\n", buffer);

			if (containsDot)
			{
				// Number is a double
				ret->elemA.subtype = operand_subtype__INT;
				ret->elemA.dValue = atof( (const char*) buffer );
			}
			else
			{
				// Number is an integer
				ret->elemA.subtype = operand_subtype__DOUBLE;
				ret->elemA.iValue = atol( (const char*) buffer );
			}

			ret->elemA.type = operand_type__CONST;
			return ret;
		}
		else
		{
			// Search for a series of parenthesis
			range_t subEqRange = searchParenthesis(buffer, stringLen);

			if (subEqRange.exists)
			{
				range_t functionRange = { .start = 0, .stop = stringLen - 1 };

				ret->elemA.type = operand_type__FUNCTION;
				ret->elemA.function = parseFunction(buffer, functionRange);

				return ret;
			}
			else
			{
				ERROR("Invalid input !! '%s'\n", buffer);

				ret->elemA.type = operand_type__NONE;
				ret->elemA.subtype = operand_subtype__NAN;

				ret->elemB.type = operand_type__NONE;
				ret->elemB.subtype = operand_subtype__NAN;

				return NULL;
			}
		}
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
		equation_t* eq2 = parseEquation(buffer, r2);

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
		equation_t* eq2 = parseEquation(buffer, r2);

		// Fill return structure
		ret->elemA.nestedEq = eq1; ret->elemA.type = operand_type__NESTED_EQ;
		ret->elemB.nestedEq = eq2; ret->elemB.type = operand_type__NESTED_EQ;

		if(idx_mul == mul_div) ret->operation = operation_type__TIMES;
		if(idx_div == mul_div) ret->operation = operation_type__DIV;
	}

	return ret;
}


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

	char* begP = strnchrf(buffer, stringLen, '('); if (begP) *begP = '\0';
	char* endP = strnchrl(buffer, stringLen, ')'); if (endP) *endP = '\0';

	// Save pointer to function's name and function's args
	char* funName_raw = buffer;
	char* funArgs_raw = buffer + (int) strlen(funName_raw) + 1;

	TRACE("Fun: %s() | Args (raw): %s\n", funName_raw, funArgs_raw);

	// Count function's arguments
	int argsCounter = 0;
	char* args[5];

	size_t argsLen_raw = strlen(funArgs_raw);

	args[argsCounter++] = funArgs_raw; // First argument starts at 0

	while (argsCounter < 5)
	{
		// Search for comma(s) while avoiding sub-equations
		int commaPos = strnfind(funArgs_raw, argsLen_raw, ',');
		if (commaPos == -1) break;

		// int prevPos = argPos[argsCounter-1];

		// Error handling
		// if (commaPos == (prevPos+1))
		// {
		// 	ERROR("Empty argument detected in equation '%s'\n", buffer);
		// 	exit(1);
		// }

		// Save argement position and replace comma with 'nul' char
		args[argsCounter++] = funArgs_raw + commaPos + 1;
		funArgs_raw[commaPos] = '\0';
	}

	// Init the returned pointer with an empty structure
	function_t* ret = new_function_t(argsCounter);

	// Then, fill the structure
	strncpy(ret->name, funName_raw, FUNCTION_NAME_MAXLEN);

	for (int i = 0; i < argsCounter; i++)
	{
		INFO("  Arg %d: %s\n", i, args[i]);

		range_t argumentRange = { .start = 0, .stop = strlen(args[i]) - 1 };
		ret->arguments[i] = parseEquation(args[i], argumentRange);
	}

	// Cleanup needed stuff, then return the structure pointer
	free(buffer);
	return ret;
}
