#ifndef __VTSP_MESH_INTEGRAL__
#define __VTSP_MESH_INTEGRAL__

#include <stdint.h>

#include "vtsp.h"

int vtsp_integrate_path(const vtsp_mesh_t *input_mesh,
			const vtsp_field_t *input_field,
			uint32_t from_node, uint32_t to_node,
			double* output);
#endif
