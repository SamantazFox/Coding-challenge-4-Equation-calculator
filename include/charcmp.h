#ifndef __CHARCMP_H__
#define __CHARCMP_H__


#include <ctype.h>


static inline short int isMathBase(char c)
{
	return c == '+' || c == '-' || c == '/' || c == '*';
}

static inline short int isMathSpecial(char c)
{
	return  c == '%' || c == '!' || c == '^';
}

static inline short int isMathConst(char c)
{
	return c == 'e' || c == 'i' || c == 'x';
}


static inline short int isNumber(char c)
{
	return isdigit(c) || c == '.';
}

static inline short int isSign(char c)
{
	return c == '+' || c == '-';
}

static inline short int isBinary(char c)
{
	return c == '0' || c == '1';
}


static inline short int isValidChar(char c)
{
	return isalnum(c) || isMathBase(c) || isMathSpecial(c) ||
		c == '(' || c == ')' ||	c == '.' || c == ',';
}


#endif  /* !__CHARCMP_H__ */
