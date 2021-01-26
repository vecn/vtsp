#ifndef __TRIANGULAR_MESH__
#define __TRIANGULAR_MESH__

#include <stdbool.h>
#include <stdint.h>

#include "delaunay_trg.h"
  
typedef struct {
	uint32_t p1, p2; /* Points forming segment */
} dms_sgm_t;


typedef struct {
	uint32_t num;
	uint32_t n_alloc;
	dms_sgm_t *data;
} dms_sgms_t;

typedef struct {
	dms_points_t vtx;
	dms_sgms_t sgm;
	dms_points_t holes;
} dms_solid_t;

typedef struct {
	uint32_t ntrg;
	uint32_t nedg;
	uint32_t nnod;
	float n1[2], n2[2], n3[2];
	bool is_s1_boundary;
	bool is_s2_boundary;
	bool is_s3_boundary;
} dms_trg_ctx_t;

typedef struct {
	void *ctx;
	uint32_t max_nodes;
	int (*should_split_trg)(void *ctx, const dms_trg_ctx_t *in, bool* out);
} dms_extra_refine_t;

typedef struct {
	dms_mesh_t mesh;
	dms_perm_t map_vtx;   /* Map solid's vertices to mesh nodes */
	dms_perm_t *map_sgm;  /* Map solid's segments to mesh edges */
} dms_xmesh_t;


int dms_verify_solid_sizeof_opmem(const dms_solid_t *input, uint32_t *output);
/*
 * With constrained Delaunay trg, check that
 *   t = 2n - 2 + 2h - b
 */
int dms_verify_solid(const dms_solid_t *input, char output_msg[100],
		     void *op_mem);

int dms_get_mesh_sizeof_opmem(const dms_solid_t *input,
			      const dms_extra_refine_t *refine,
			      uint32_t *output);
/*
 * e = 3n + 3h - 3 - b
 * t = 2n + 2h - 2 - b
 *
 * e <- num of edges
 * t <- num of triangles
 * n <- num of nodes          (Defined in input)
 * h <- num of holes          (Defined in input)
 * b <- Vertices in boundary  (Estimated from input, min is 3)
 */
int dms_get_mesh_sizeof_output(const dms_solid_t *input,
			       const dms_extra_refine_t *refine,
			       uint32_t *output);
int dms_get_mesh(const dms_solid_t *input,
		 const dms_extra_refine_t *refine,
		 void *output, void *op_mem);
int dms_get_mesh_ref_output(void *output, dms_xmesh_t **ref);

#endif
