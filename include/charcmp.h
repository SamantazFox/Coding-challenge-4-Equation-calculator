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

static inline short int isValidChar(char c)
{
	return isalnum(c) || isMathBase(c) || isMathSpecial(c) ||
		c == '(' || c == ')' ||	c == '.' || c == ',';
}


#endif  /* !__CHARCMP_H__ */
