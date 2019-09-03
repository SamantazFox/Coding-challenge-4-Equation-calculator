#include "parser.h"

#include "charcmp.h"
#include "stdbool.h"
#include "assert.h"


#define min(a,b) ((a<b) ? a : b)
#define max(a,b) ((a>b) ? a : b)


typedef struct
{
	unsigned int start;
	unsigned int stop;
	bool exists;
}
range_t;


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

	for (int i = 0; i < len; i++) if (buf[i] == item) { pos = i; break; }

	if (subRange.exists && pos >= subRange.start && pos <= subRange.stop)
		return strnfind(buf + subRange.stop, len - 1 - subRange.stop, item);

	return pos;
}


equation_t* parseEquation(char* stringToParse, size_t stringLen)
{
	// Init the returned pointer with an empty structure
	equation_t* ret = new_equation_t();

	// Copy sub-equation string to new buffer
	char buffer[stringLen+1];
	strncpy(buffer, stringToParse, stringLen);
	buffer[stringLen] = '\0';


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
		// Search for a series of parenthesis
		range_t subEqRange = searchParenthesis(buffer, stringLen);

		if (!subEqRange.exists)
		{
			// Parse the number
			ret->elemA.type    = operand_type__CONST;
			ret->elemA.subtype = operand_subtype__DOUBLE;
			ret->elemA.dValue  = 0;

			// ret->elemB.type    = operand_type__CONST;
			// ret->elemB.subtype = operand_subtype__DOUBLE;
			// ret->elemB.dValue  = 0;

			INFO("Found an number !! '%s'\n", buffer);
			return ret;
		}
		else
		{
			ret->elemA.type     = operand_type__FUNCTION;
			ret->elemA.function = parseFunction(buffer, stringLen);
			return ret;
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
		range_t r2 = { .start = add_sub + 1, .stop = stringLen };

		// Parse sub-equations
		equation_t* eq1 = parseEquation(buffer + r1.start, r1.stop - r1.start + 1);
		equation_t* eq2 = parseEquation(buffer + r2.start, r2.stop - r2.start + 1);

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
		range_t r2 = { .start = mul_div + 1, .stop = stringLen };

		// Parse sub-equations
		equation_t* eq1 = parseEquation(buffer + r1.start, r1.stop - r1.start);
		equation_t* eq2 = parseEquation(buffer + r2.start, r2.stop - r2.start);

		// Fill return structure
		ret->elemA.nestedEq = eq1; ret->elemA.type = operand_type__NESTED_EQ;
		ret->elemB.nestedEq = eq2; ret->elemB.type = operand_type__NESTED_EQ;

		if(idx_mul == mul_div) ret->operation = operation_type__TIMES;
		if(idx_div == mul_div) ret->operation = operation_type__DIV;
	}

	return ret;
}


function_t* parseFunction(char* stringToParse, size_t stringLen)
{
	// Copy sub-equation string to new buffer
	// Then replace surrounding parenthesis with '\0'
	char* buffer = strsafecpy(stringToParse, stringLen);

	TRACE("Function: %s\n", buffer);

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
		ret->arguments[i] = parseEquation(args[i], strlen(args[i]));
	}

	// Cleanup needed stuff, then return the structure pointer
	free(buffer);
	return ret;
}
