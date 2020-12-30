#ifndef __VTSP_H__
#define __VTSP_H__

#include <stdint.h>

#include "vtsp_types.h"
#include "vtsp_depend.h"


int vtsp_solve_sizeof_opmem(const vtsp_points_t *input, uint32_t *output);

int vtsp_solve(const vtsp_points_t *input, vtsp_perm_t *output,
	       vtsp_depend_t *depend, void *op_mem);

#endif
