#include <stdint.h>

#include "vtsp.h"

#define MIN_POINTS 3
#define MAX_POINTS 20000000


#define TRY(function_call) \
	do { \
		int status = (function_call); \
		if (0 != status) { \
			return status; \
		} \
	} while(0)

#define TRY_CATCH(function_call, do_catch) \
	do { \
		int status = (function_call); \
		if (0 != status) { \
			(do_catch); \
		} \
	} while(0)


static int validate_input(const vtsp_points_t *input,
			  const vtsp_depend_t *depend);
static int log(const vtsp_depend_t *depend, const char *msg);
static int get_convex_envelope(const vtsp_points_t *input, vtsp_perm_t *output,
			       vtsp_depend_t *depend, void *op_mem);

uint64_t vtsp_sizeof_operational_memory()
{
	return 1;
}

vtsp_status_t vtsp_solve(const vtsp_points_t *input, vtsp_perm_t *output,
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
	if (input->points < MIN_POINTS) {
		sprintf(msg, "Num of points must be at least %i", MIN_POINTS);
	} else if (input->points > MAX_POINTS) {
		sprintf(msg, "Num of points is bigger than max %s", MAX_POINTS);
	}
	
	if (strlen(msg) > 0) {
		TRY( log(depend, msg) );
		return MALFORMED_INPUT;
	}
	return SUCCESS;
}

static int log(const vtsp_depend_t *depend, const char *msg)
{
	TRY_CATCH(
		depend->logger.log(depend->logger.ctx, msg),
		return ERROR
	);
	return SUCCESS;
}

static int solve(const vtsp_points_t *input, vtsp_perm_t *output,
		 vtsp_depend_t *depend, void *op_mem)
{
	TRY( get_convex_envelope(input, output, depend) );
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
	TRY_CATCH(
		depend->envelope.get_convex_envelope(depend->envelope.ctx,
						     input, output),
	        {
			TRY( log(depend, "Error computing convex envelope.");
			return ERROR;
		}
	);
}

		
