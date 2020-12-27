#ifndef __VTSP_H__
#define __VTSP_H__

#include <stdint.h>

#include "vtsp_types.h"
#include "vtsp_depend.h"


size_t vtsp_sizeof_operational_memory(void);

int vtsp_solve(const vtsp_points_t *input, vtsp_perm_t *output,
	       vtsp_depend_t *depend, void *op_mem);

#endif
