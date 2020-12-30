#ifndef __DELAUNAY_TRIANGULATION__
#define __DELAUNAY_TRIANGULATION__

#include <stdint.h>

#define DMS_NO_ADJ (-1)

typedef struct {
	float x;
	float y;
} dms_point_t;

typedef struct {
	uint32_t num;
	dms_point_t *data;
} dms_points_t;

typedef struct {
	uint32_t p1, p2;
} dms_sgm_t;

typedef struct {
	uint32_t n1, n2; /* Nodes connecting mesh edge */
	uint32_t t1, t2; /* Triangles adjacent to mesh */
} dms_edge_t;

typedef struct {
	uint32_t num;
	dms_edge_t *data;
} dms_edges_t;

typedef struct {
	uint32_t n1, n2, n3; /* Nodes forming triangle (CCW) */
	uint32_t t1, t2, t3; /* Adjacent triangles (CCW) */
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
	dms_points_t nodes;
	dms_edges_t edges;
	dms_trgs_t trgs;
} dms_mesh_t;

int dms_get_convex_envelope_sizeof_opmem(const dms_points_t *input, uint32_t *output);
int dms_get_convex_envelope(const dms_points_t *input,
			    dms_perm_t *output,
			    void *op_mem);

int dms_get_delaunay_trg_sizeof_opmem(const dms_points_t *input, uint32_t *output);
int dms_get_delaunay_trg(const dms_points_t *input,
			 const dms_edges_t *constraints,
			 dms_mesh_t *output,
			 void *op_mem);

#endif
