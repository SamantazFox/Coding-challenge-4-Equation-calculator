#include "parser.h"
#include "charcmp.h"


equation_t* parseEquation(char* start, size_t len)
{
	equation_t* ret = NULL; new_equation_t();

	// Copy sub-equation string to new buffer
	char buffer[len+1];
	strncpy(buffer, start, len);
	buffer[len] = '\0';

	DEBUG("Sub-equation: %s\n", buffer);

	// Determine where is the closest element
	size_t m = strcspn(buffer, "+-");
	size_t p = strcspn(buffer, "(" );

	size_t min = (m < p) ? m : p;

	if (m < p && m < len)
	{
		equation_t* a = parseEquation(buffer, min);
		equation_t* b = parseEquation(buffer+min+1, len-(min+1) );
	}

	// printf("c1 = %d / c2 = %d\n", (int) m, (int) n);

	// char* c1 = memchr(buffer, '/', len);
	// char* c2 = memchr(buffer, '*', len);

	// printf("c1 = %d / c2 = %d \n", (int) (c1-buffer), (int) (c2-buffer) );

	char*  beginSub  = NULL;
	size_t subLength = 0;

	int parenthesisCount = 0;

	for (int i = 0; i < len; i++)
	{
		if (buffer[i] == '(') parenthesisCount++;
		if (buffer[i] == ')') parenthesisCount--;

		if (parenthesisCount == 1 && beginSub == NULL)
			beginSub = buffer + i + 1;

		if (parenthesisCount == 0 && beginSub != NULL)
		{
			subLength = (buffer + i) - beginSub;
			parseEquation(beginSub, subLength);
			beginSub = NULL;
		}
	}

	// if (beginSub != NULL && subLength > 0)
	// 	ret = parseEquation(beginSub, subLength);

	return ret;
}
