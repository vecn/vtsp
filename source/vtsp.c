#include <stdint.h>

#include "vtsp.h"


uint64_t vtsp_sizeof_operational_memory()
{
	return 1;
}

vtsp_status_t vtsp_solve(const vtsp_points_t *input, vtsp_perm_t *output,
			 vtsp_depend_t *dependencies)
{
	if (0 != validate_input(input, dependencies)) {
		return MALFORMED_INPUT;
	}
	if (0 != solve(input, output, dependencies)) {
		return ERROR;
	}
	return SUCCESS;
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

		
