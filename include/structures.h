#ifndef __STRUCTURES_H__
#define __STRUCTURES_H__


#include <stdint.h>


#define FUNCTION_NAME_MAXLEN  10


/*
 * ENUMERATES
*/

typedef enum
{
	operand_type__NONE,
	operand_type__CONST,
	operand_type__NESTED_EQ,
	operand_type__FUNCTION
}
operand_type_e;

typedef enum
{
	operand_subtype__NAN,
	operand_subtype__INT,
	operand_subtype__UINT,
	operand_subtype__DOUBLE
}
operand_subtype_e;


typedef enum
{
	operation_type__NONE,
	operation_type__PLUS,
	operation_type__MINUS,
	operation_type__TIMES,
	operation_type__DIV,
	operation_type__MOD,
	operation_type__POW
}
operation_type_e;



/*
 * DATA STRUCURES
*/

typedef struct fun_struct function_t;
typedef struct ope_struct operand_t;
typedef struct equ_struct equation_t;


typedef struct fun_struct
{
	char        name[FUNCTION_NAME_MAXLEN];
	uint8_t     argCount;
	equation_t* arguments[];
}
function_t;


typedef struct ope_struct
{
	// (Sub)Type is used to determine the contents of this structure
	operand_type_e    type;
	operand_subtype_e subtype;

	// Numeric value for this operand (64 bits)
	// In the case of nested eq/fun, it contains the said eq/fun's result
	union
	{
		double   dValue;
		int64_t  iValue;
		uint64_t uValue;
	};

	// Operand can be a nedsted fun/eq
	function_t* function;
	equation_t* nestedEq;

}
operand_t;

typedef struct equ_struct
{
	// Operands
	operand_t elemA;
	operand_t elemB;

	// Operation done between A & B
	operation_type_e operation;
}
equation_t;


#endif  /* !__STRUCTURES_H__ */
