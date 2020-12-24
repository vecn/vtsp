#include <stdlib.h>
#include <stdint.h>

#include "try_macros.h"
#include "vtsp.h"

enum {
	ERROR_MALLOC = 100
};


static int load_points(vtsp_points_t *input);
static int allocate_output(const vtsp_points_t *input, vtsp_perm_t *output);
static int set_dependencies(vtsp_depend_t *depend);
	
int main(int argc, const char* argv[])
{
	
	uint64_t memsize = vtsp_sizeof_operational_memory();
	void *mem;
	TRY_PTR( malloc(memsize), mem, ERROR_MALLOC );

	vtsp_points_t input;
	TRY( load_points(&input) );
	
	
	vtsp_perm_t output;
	TRY( allocate_output(&input, &output) );

	vtsp_depend_t depend;
	TRY( set_dependencies(&depend) );
	
	TRY( vtsp_solve(&input, &output, &depend, mem) );
	return SUCCESS;
}

static int load_points(vtsp_points_t *input)
{
	size_t size = input->num * 2 * sizeof(*(input->coord));
	TRY_PTR( malloc(size), input->coord, ERROR_MALLOC );

	int i;
	for (i = 0; i < input->num; i++) {
		input->coord[i * 2] = i;
		input->coord[i*2+1] = i*i;
	}
	
	return SUCCESS;
}

static int allocate_output(const vtsp_points_t *input, vtsp_perm_t *output)
{
	output->num = input->num;
	
	size_t size = output->num * sizeof(*(output->index));
	TRY_PTR( malloc(size), output->index, ERROR_MALLOC );

	return SUCCESS;
}

static int set_dependencies(vtsp_depend_t *depend)
{
	//depend->logger;
	//depend->drawer;
	//depend->reporter;
	//depend->envelope;
	//depend->mesher;
	//depend->heat;
	//depend->integral;
	return SUCCESS;
}
