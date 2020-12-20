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


static int validate_input(const vtsp_points_t *input,
			  const vtsp_depend_t *depend);


uint64_t vtsp_sizeof_operational_memory()
{
	return 1;
}

vtsp_status_t vtsp_solve(const vtsp_points_t *input, vtsp_perm_t *output,
			 vtsp_depend_t *depend)
{
	TRY( validate_input(input, depend) );
        TRY( solve(input, output, depend) );
	return SUCCESS;
}

static int validate_input(const vtsp_points_t *input,
			  const vtsp_depend_t *depend)
{
	if (input->points < MIN_POINTS) {
		log(depend, "Num of points must be at least 3.");
		return MALFORMED_INPUT;
	}
	
	if (input->points > MAX_POINTS) {
		log(depend, "Num of points is bigger than max.");
		return MALFORMED_INPUT;
	}
	return SUCCESS;
}

static void log(const vtsp_depend_t *depend, const char *msg)
{
	
}

static int solve()
{
	path = get_convex_envelope();
	mesh = get_mesh();
	set_conditions(input, mesh);
	field = solve_heat();
	while (more_points()) {
		add_point(path, mesh);
	}
	return SUCCESS;
}

static int add_point()
{
}

		
