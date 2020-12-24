#ifndef __TRY_MACROS_H__
#define __TRY_MACROS_H__

#define TRY(function_call)			\
	do { \
		int __status__ = (function_call); \
		if (0 != __status__) {     \
			return __status__; \
		} \
	} while(0)

#define TRY_SET(function_call, error_code)    \
	do { \
		if (0 != (function_call)) {   \
			return (error_code);  \
		} \
	} while(0)


#define TRY_NONEG(function_call, error_code)  \
	do { \
		if (0 > (function_call)) {    \
			return (error_code);  \
		} \
	} while(0)

#define TRY_PTR(function_call, ptr, error_code)	\
	do { \
		(ptr) = (function_call);      \
		if (0 >= (ptr)) {	      \
			return (error_code);  \
		} \
	} while(0)


#endif
