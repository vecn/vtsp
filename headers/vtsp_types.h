#ifndef __VTSP_TYPES_H__
#define __VTSP_TYPES_H__

#include <stdint.h>

enum {
	SUCCESS = 0,
	MALFORMED_INPUT,
	ERROR = 100000
};

typedef struct {
	float x;
	float y;
} vtsp_point_t;

typedef struct {
	uint32_t num;
	uint32_t n_alloc;
	vtsp_point_t *pts;
} vtsp_points_t;

typedef struct {
	uint32_t num;
	uint32_t n_alloc;
	uint32_t *index;
} vtsp_perm_t;

typedef struct {
	uint32_t num;
	uint32_t n_alloc;
	float *values;
} vtsp_field_t;

typedef struct {
	uint32_t n1, n2, n3;
} vtsp_trg_t;

typedef struct {
	uint32_t num;
	uint32_t n_alloc;
	vtsp_trg_t* trgs;
} vtsp_trgs_t;

typedef struct {
	vtsp_points_t nodes;
	vtsp_trgs_t adj;
	vtsp_perm_t map_vtx;
} vtsp_mesh_t;

#endif
