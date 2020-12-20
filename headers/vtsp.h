#ifndef __VTSP_H__
#define __VTSP_H__

#include <stdint.h>

#include "vtsp_types.h"

typedef struct {
	void *operation_memory;
	int (*debug_log)(void* ctx, const char* msg);
	int (*debug_draw_state)(void ctx, const vtsp_points_t *input,
				const vtsp_points_t *mesh,
				const vtsp_field_t *field,
				const vtsp_perm_t *path);
	int (*get_convex_envelope)(void* ctx, const vtsp_points_t *input,
				   vtsp_perm_t *output);
	int (*get_mesh)(void* ctx, const vtsp_points_t *input,
			vtsp_mesh_t *mesh)
	int (*solve_heat)(void* ctx, const vtsp_mesh_t *input,
			  vtsp_field_t *output);
	int (*integrate_path)(const vtsp_field_t *field,
			      const vtsp_points_t *points,
			      float* output);
} vstp_depend_t;

uint64_t vtsp_sizeof_operational_memory();

vtsp_status_t vtsp_solve(const vtsp_points_t *input, vtsp_perm_t *output,
			 vtsp_depend_t *dependencies);

#endif
