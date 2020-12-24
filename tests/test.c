#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "try_macros.h"
#include "vtsp.h"

enum {
	ERROR_MALLOC = 100
};


static int load_points(vtsp_points_t *input);
static int allocate_output(const vtsp_points_t *input, vtsp_perm_t *output);

static int bind_dependencies(vtsp_depend_t *depend);
static int bind_logger(vtsp_binding_logger_t *logger);
static int bind_drawer(vtsp_binding_drawer_t *drawer);
static int bind_reporter(vtsp_binding_reporter_t *reporter);
static int bind_envelope(vtsp_binding_envelope_t *envelope);
static int bind_mesher(vtsp_binding_mesher_t *mesher);
static int bind_heat(vtsp_binding_heat_t *heat);
static int bind_integral(vtsp_binding_integral_t *integral);

static int bind_log(void *ctx, const char *msg);

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
	TRY( bind_dependencies(&depend) );
	
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

static int bind_dependencies(vtsp_depend_t *depend)
{
	TRY( bind_logger(&(depend->logger)) );
	TRY( bind_drawer(&(depend->drawer)) );
	TRY( bind_reporter(&(depend->reporter)) );
	TRY( bind_envelope(&(depend->envelope)) );
	TRY( bind_mesher(&(depend->mesher)) );
	TRY( bind_heat(&(depend->heat)) );
	TRY( bind_integral(&(depend->integral)) );
	return SUCCESS;
}

static int bind_logger(vtsp_binding_logger_t *logger)
{
	logger->ctx = "logfile.txt";
	logger->log = &bind_log;
	return SUCCESS;
}

static int bind_drawer(vtsp_binding_drawer_t *drawer)
{
	// Bind
	return SUCCESS;
}

static int bind_reporter(vtsp_binding_reporter_t *reporter)
{
	// Bind
	return SUCCESS;
}

static int bind_envelope(vtsp_binding_envelope_t *envelope)
{
	// Bind
	return SUCCESS;
}

static int bind_mesher(vtsp_binding_mesher_t *mesher)
{
	// Bind
	return SUCCESS;
}

static int bind_heat(vtsp_binding_heat_t *heat)
{
	// Bind
	return SUCCESS;
}

static int bind_integral(vtsp_binding_integral_t *integral)
{
	// Bind
	return SUCCESS;
}

static int bind_log(void *ctx, const char *msg) {
	FILE *fp;
	TRY_PTR(  fopen((char*) ctx, "a"), fp, ERROR );
	
	TRY_NONEG( fprintf(fp, "%s\n", msg), ERROR );

	TRY( fclose(fp) );
	return SUCCESS;
}
