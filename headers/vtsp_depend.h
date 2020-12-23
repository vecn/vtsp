#ifndef __VTSP_DEPEND_H__
#define __VTSP_DEPEND_H__

#include <stdint.h>

#include "vtsp_types.h"

typedef struct {
	void *ctx;
	int (*log)(void *ctx, const char *msg);
} vtsp_binding_logger_t;


typedef struct {
	void *ctx;
	int (*draw_state)(void *ctx, const vtsp_points_t *points,
			  const vtsp_mesh_t *mesh,
			  const vtsp_field_t *field,
			  const vtsp_perm_t *path);
} vtsp_binding_drawer_t;

typedef struct {
	void *ctx;
	int (*report_progress)(void *ctx, float percent);
} vtsp_binding_reporter_t;

typedef struct {
	void *ctx;
	int (*get_convex_envelope)(void* ctx, const vtsp_points_t *input,
				   vtsp_perm_t *output);
} vtsp_binding_envelope_t;

typedef struct {
	void *ctx;
	int (*get_mesh)(void* ctx, const vtsp_points_t *input,
			vtsp_mesh_t *mesh);
} vtsp_binding_mesher_t;

typedef struct {
	void *ctx;
	int (*solve_heat)(void* ctx, const vtsp_mesh_t *input,
			  vtsp_field_t *output);
} vtsp_binding_heat_t;

typedef struct {
	void *ctx;
	int (*integrate_path)(const vtsp_field_t *field,
			      const vtsp_points_t *points,
			      float* output);
} vtsp_binding_integral_t;

typedef struct {
	vtsp_binding_logger_t logger;
	vtsp_binding_drawer_t drawer;
	vtsp_binding_reporter_t reporter;
	vtsp_binding_envelope_t envelope;
	vtsp_binding_mesher_t mesher;
	vtsp_binding_heat_t heat;
	vtsp_binding_integral_t integral;
} vtsp_depend_t;

#endif
