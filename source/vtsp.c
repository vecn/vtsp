#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "vtsp.h"
#include "try_macros.h"

#define MIN_POINTS 3
#define MAX_POINTS 20000000


enum {
	ERROR_INTERNAL = ERROR,
	ERROR_SPRINTF,
	ERROR_WRITE_LOG
};

static int validate_input(const vtsp_points_t *input,
			  const vtsp_depend_t *depend);
static int write_log(const vtsp_depend_t *depend, const char *msg);
static int solve(const vtsp_points_t *input, vtsp_perm_t *output,
		 vtsp_depend_t *depend, void *op_mem);
static int get_convex_envelope(const vtsp_points_t *input, vtsp_perm_t *output,
			       vtsp_depend_t *depend, void *op_mem);
static int log_perm(vtsp_depend_t *depend,  vtsp_perm_t *output,
		    const char *prefix);

int vtsp_solve_sizeof_opmem(const vtsp_points_t *input, uint32_t *output)
{
	*output = 1;
	return SUCCESS;
}

int vtsp_solve(const vtsp_points_t *input, vtsp_perm_t *output,
			 vtsp_depend_t *depend, void *op_mem)
{
	TRY( validate_input(input, depend) );
        TRY( solve(input, output, depend, op_mem) );
	return SUCCESS;
}

static int validate_input(const vtsp_points_t *input,
			  const vtsp_depend_t *depend)
{
	char msg[100];
	msg[0] = 0;
	if (input->num < MIN_POINTS) {
		TRY_NONEG( sprintf(msg, "Num of points must be at least %i", MIN_POINTS),
			   ERROR_SPRINTF );
	} else if (input->num > MAX_POINTS) {
		TRY_NONEG( sprintf(msg, "Num of points is bigger than max %i", MAX_POINTS),
			   ERROR_SPRINTF );
	}
	
	if (strlen(msg) > 0) {
		TRY( write_log(depend, msg) );
		return MALFORMED_INPUT;
	}
	return SUCCESS;
ERROR_SPRINTF:
	return ERROR_SPRINTF;
}

static int write_log(const vtsp_depend_t *depend, const char *msg)
{
	TRY_GOTO( depend->logger.log(depend->logger.ctx, msg),
		  ERROR_WRITE_LOG );
	return SUCCESS;
ERROR_WRITE_LOG:
	return ERROR_WRITE_LOG;
}

static int solve(const vtsp_points_t *input, vtsp_perm_t *output,
		 vtsp_depend_t *depend, void *op_mem)
{
	TRY( get_convex_envelope(input, output, depend, op_mem) );
	//mesh = get_mesh();
	//set_conditions(input, mesh);
	//field = solve_heat();
	//while (more_points()) {
	//	add_point(path, mesh);
	//}
	return SUCCESS;
}

static int get_convex_envelope(const vtsp_points_t *input, vtsp_perm_t *output,
			       vtsp_depend_t *depend, void *op_mem)
{
	int status = depend->envelope.get_convex_envelope(depend->envelope.ctx,
							  input, output);
	char msg[100];
	if (0 != status) {
		TRY_NONEG( sprintf(msg, "Error computing convex envelope (code %i).", status),
			   ERROR_SPRINTF );
		TRY( write_log(depend, msg) );
		return ERROR;
	}
	TRY_NONEG( sprintf(msg, "Convex envelope has %i points.", output->num),
		   ERROR_SPRINTF );
	TRY( log_perm(depend, output, "Indices forming convex envelope") );
	TRY( write_log(depend, msg) );
	return SUCCESS;
ERROR_SPRINTF:
	return ERROR_SPRINTF;
}

static int log_perm(vtsp_depend_t *depend,  vtsp_perm_t *perm,
		    const char *prefix)
{
	char msg[100];
	uint32_t i;
	TRY_NONEG( sprintf(msg, "%s:", prefix), ERROR_SPRINTF );
	TRY( write_log(depend, msg) );
	for (i = 0; i < perm->num; i++) {
		TRY_NONEG( sprintf(msg, "  %u", perm->index[i]), ERROR_SPRINTF );
		TRY( write_log(depend, msg) );
	}
	return SUCCESS;
ERROR_SPRINTF:
	return ERROR_SPRINTF;
}
