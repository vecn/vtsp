#ifndef __TRIANGULAR_MESH__
#define __TRIANGULAR_MESH__

#include <stdbool.h>
#include <stdint.h>

#include "delaunay_trg.h"
  
typedef struct {
	dms_points_t vtx;
	dms_sgms_t sgm;
	dms_points_t holes;
} dms_solid_t;

typedef struct {
	uint32_t ntrg;
	uint32_t nvtx;
	float v1[2], v2[2], v3[2];
	bool is_s1_boundary;
	bool is_s2_boundary;
	bool is_s3_boundary;
} dms_trg_ctx_t;

typedef struct {
	void *ctx;
	int (*should_split_trg)(void *ctx, const dms_trg_ctx_t *in, bool* out),
} dms_extra_refine_t;

typedef struct {
	dms_solid_t solid;
	dms_mesh_t mesh;
	dms_perm_t map_vtx;  /* Map solid's vertices to mesh nodes */
	dms_perm_t *map_sgm; /* Map solid's segments to mesh edges */
} dms_xmesh_t;

int dms_get_mesh(const dms_solid_t *input,
		 const dms_extra_refine_t *refine,
		 const dms_xmesh_t *output);

#endif
