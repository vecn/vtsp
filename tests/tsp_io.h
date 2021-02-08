#ifndef __TSP_IO_H__
#define __TSP_IO_H__

#include <stdint.h>

#include "vtsp.h"

int vtsp_read_problem_npts(const char *input_filename, uint32_t *output);
int vtsp_read_problem(const char *input_filename, vtsp_points_t *output);
int vtsp_read_tour(const char *input_filename, vtsp_perm_t *output);
int vtsp_write_tour(const vtsp_perm_t *input, const char *output_filename);

#endif
