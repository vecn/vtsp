#ifndef __FEM_SOLVE_HEAT__
#define __FEM_SOLVE_HEAT__

#include <stdint.h>

typedef struct {
	float x, y;
} fem_node_t;

typedef struct {
	uint32_t n1, n2, n3;
} fem_elem_t;

typedef struct {
	uint32_t n_nodes;
	fem_node_t *nodes;
	uint32_t n_elems;
	fem_elem_t *elems;
} fem_mesh_t;

typedef struct {
	uint32_t n;
	uint32_t *index;
	float *values;
} fem_fix_nodes_t;

typedef struct {
	uint32_t elem;
	uint32_t n1, n2;
} fem_edge_t;

typedef struct {
	uint32_t n;
	fem_edge_t *edges;
	float *values;	
} fem_fix_edges_t;

typedef struct {
	fem_fix_nodes_t on_nodes;
	fem_fix_edges_t on_edges;

} fem_fix_temperature_t;

typedef struct {
	fem_fix_nodes_t on_nodes;
	fem_fix_edges_t on_edges;

} fem_fix_flux_t;

typedef struct {
	fem_fix_temperature_t temp;
	fem_fix_flux_t flux;
} fem_cond_t;

typedef struct {
	float diffusion;
	fem_mesh_t mesh;
	fem_cond_t cond;
} fem_input_t;

int fem_solve_heat_sizeof_opmem(const fem_input_t *input,
				const uint32_t *memsize);

int fem_solve_heat(const fem_input_t *input,
		   const float *output,
		   void *opmem);

#endif
