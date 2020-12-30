#ifndef __TRY_MACROS_H__
#define __TRY_MACROS_H__

#define TRY(function_call)			\
	do { \
		int __status__ = (function_call); \
		if (0 != __status__) {     \
			return __status__; \
		} \
	} while(0)


#define TRY_GOTO(function_call, LABEL)       \
	do { \
		int __status__ = (function_call); \
		if (0 != __status__) {     \
			goto LABEL;        \
		} \
	} while(0)


#define TRY_NONEG(function_call, LABEL)       \
	do { \
		if (0 > (function_call)) {    \
			goto LABEL;           \
		} \
	} while(0)

#define TRY_PTR(function_call, ptr, LABEL)    \
	do { \
		(ptr) = (function_call);      \
		if (0 >= (ptr)) {	      \
			goto LABEL;           \
		} \
	} while(0)

#define THROW(condition, error)          \
	do { \
		if (condition) {         \
			return (error);  \
		} \
	} while(0)

#endif
