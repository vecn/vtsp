#ifndef __DELAUNAY_TRIANGULATION__
#define __DELAUNAY_TRIANGULATION__

#include <stdint.h>

enum {
	SUCCESS = 0,
	BAD_INPUT,
	ERROR
}

typedef struct {
	float x;
	float y;
} dms_point_t;

typedef struct {
	uint32_t num;
	dms_point_t *data;
} dms_points_t;

typedef struct {
	uint32_t v1, v2;
} dms_sgm_t;

typedef struct {
	uint32_t num;
	dms_sgm_t *data;
} dms_sgms_t;

typedef struct {
	uint32_t v1, v2, v3; /* Vertices */
	uint32_t t1, t2, t3; /* Adjacent triangles */
} dms_trg_t;

typedef struct {
	uint32_t num;
	dms_trg_t *data;
} dms_trgs_t;

typedef struct {
	/* Subset's permutation of geom elems */
	uint32_t num;
	uint32_t *index;
} dms_perm_t;

typedef struct {
	dms_points_t vtx;
	dms_sgms_t sgm;
	dms_trgs_t trg;
} dms_mesh_t;

int dms_get_convex_envelope(const dms_points_t *input, dms_perm_t *output);

int dms_get_delaunay_trg(const dms_points_t *input, dms_mesh_t *output);

int dms_get_constrained_delaunay_trg(const dms_points_t *input,
				     const dms_sgms_t *constraints,
				     dms_mesh_t *output);

#endif
