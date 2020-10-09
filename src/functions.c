#include "functions.h"


const function_def_t known_functions_g[] = {

	// Trigonometric functions
	{ "cos" , 1, &cos  },
	{ "sin" , 1, &sin  },
	{ "tan" , 1, &tan  },
	{ "acos", 1, &acos },
	{ "asin", 1, &asin },
	{ "atan", 1, &atan },

	// Hyperbolic functions
	{ "cosh" , 1, &cosh  },
	{ "sinh" , 1, &sinh  },
	{ "tanh" , 1, &tanh  },
	{ "acosh", 1, &acosh },
	{ "asinh", 1, &asinh },
	{ "atanh", 1, &atanh },

	// Exponential and logarithmic functions
	{ "exp"  , 1, &exp   },
	{ "log"  , 1, &log   },
	{ "log10", 1, &log10 },
	{ "exp2" , 1, &exp2  },
	{ "expm1", 1, &expm1 },
	{ "log1p", 1, &log1p },
	{ "log2" , 1, &log2  },
	{ "logb" , 1, &logb  },

	// Power functions
	{ "sqrt" , 1, &sqrt  },
	{ "cbrt" , 1, &cbrt  },
	{ "pow"  , 2, NULL,  &pow   },
	{ "hypot", 2, NULL,  &hypot },

	// Rounding and remainder functions
	{ "ceil" , 1, &ceil  },
	{ "floor", 1, &floor },
	{ "trunc", 1, &trunc },
	{ "round", 1, &round },
	{ "rint" , 1, &rint  },
	{ "fmod" , 2, NULL,  &fmod  },

	// Error functions
	{ "erf" , 1, &erf  },
	{ "erfc", 1, &erfc },

	// Gamma functions
	{ "tgamma", 1, &tgamma },
	{ "lgamma", 1, &lgamma },

	// Other functions
	{ "abs" , 1, &fabs },
	{ "min" , 2, NULL, &fmin },
	{ "max" , 2, NULL, &fmax },
	{ "fdim", 2, NULL, &fdim },

};

const size_t known_functions_length_g =
	sizeof(known_functions_g) / sizeof(function_def_t);
