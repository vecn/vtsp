#ifndef __VTSP_H__
#define __VTSP_H__

#include <stdint.h>

#include "vtsp_types.h"
#include "vtsp_depend.h"


uint64_t vtsp_sizeof_operational_memory();

vtsp_status_t vtsp_solve(const vtsp_points_t *input, vtsp_perm_t *output,
			 vtsp_depend_t *depend, void *op_mem);

#endif