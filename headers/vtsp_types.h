#ifndef __VTSP_TYPES_H__
#define __VTSP_TYPES_H__

#include <stdint.h>

typedef enum {
	SUCCESS = 0,
	MALFORMED_INPUT,
	ERROR = 100000
} vtsp_status_t;

typedef struct {
	uint32_t num;
	float *coord;
} vtsp_points_t;

typedef struct {
	uint32_t num;
	uint32_t *index;
} vtsp_perm_t;

typedef struct {
	uint32_t num;
	float *values;
} vtsp_field_t;

typedef struct {
	vtsp_points_t points;
	uint32_t ntrg;
	uint32_t *trg_conn;
} vtsp_mesh_t;

#endif
