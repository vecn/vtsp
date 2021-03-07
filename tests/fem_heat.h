#ifndef __FEM_SOLVE_HEAT__
#define __FEM_SOLVE_HEAT__

#include <stdint.h>

typedef struct {
	float x, y;
} fem_node_t;

typedef struct {
	uint32_t n1, n2, n3;
}

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
} fem_sgm_t;

typedef struct {
	uint32_t n;
	fem_sgm_t *elem;
	float *values;	
} fem_fix_sgms_t;

typedef struct {
	fem_fix_nodes_t *on_nodes;
	fem_fix_sgms_t *on_sgms;

} fem_fix_temperature_t;

typedef struct {
	fem_fix_nodes_t *on_nodes;
	fem_fix_sgms_t *on_sgms;

} fem_fix_flux_t;

typedef struct {
	fem_fix_temperature_t *t;
	fem_fix_flux_t *flux;
} fem_cond_t;

typedef struct {
	float diffusion;
	fem_mesh_t *mesh;
	fem_cond_t *cond;
} fem_input_t;

int fem_solve_heat_sizeof_opmem(const fem_input_t *input);

int fem_solve_heat(const fem_input_t *input,
		   const float *output,
		   void *opmem);

#endif
