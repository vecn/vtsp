#include <stdint.h>
#include <math.h>

#include "vtsp_mesh_integral.h"


int vtsp_integrate_path(const vtsp_mesh_t *input_mesh,
			const vtsp_field_t *input_field,
			uint32_t from_node, uint32_t to_node,
			double* output)
{
	// PENDING: Returning distance as a mock

	vtsp_point_t p1 = input_mesh.nodes.pts[from_node];
	vtsp_point_t p2 = input_mesh.nodes.pts[to_node];
	double xd = p2.x - p1.x;
	double yd = p2.y - p1.y;
	*output = sqrt( xd*xd + yd*yd);
	return SUCCESS;
}
