#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "try_macros.h"
#include "vtsp.h"

enum {
	ERROR_MALLOC = 100
};

typedef struct {
	float progress100;
} state_t;

static int state_init(state_t *state);
static int state_clean(state_t *state);
static int load_points(vtsp_points_t *input);
static int allocate_output(const vtsp_points_t *input, vtsp_perm_t *output);

static int bind_dependencies(vtsp_depend_t *depend, state_t *state);
static int bind_logger(vtsp_binding_logger_t *logger);
static int bind_drawer(vtsp_binding_drawer_t *drawer);
static int bind_reporter(vtsp_binding_reporter_t *reporter, float *progress100);
static int bind_envelope(vtsp_binding_envelope_t *envelope);
static int bind_mesher(vtsp_binding_mesher_t *mesher);
static int bind_heat(vtsp_binding_heat_t *heat);
static int bind_integral(vtsp_binding_integral_t *integral);

static int bind_log(void *ctx, const char *msg);
static int bind_draw_state(void *ctx, const vtsp_points_t *points,
			   const vtsp_mesh_t *mesh, const vtsp_field_t *field,
			   const vtsp_perm_t *path);
static int bind_report_progress(void *ctx, float percent);
static int bind_get_convex_envelope(void* ctx, const vtsp_points_t *input,
				    vtsp_perm_t *output);
static int bind_get_mesh(void* ctx, const vtsp_points_t *input,
			 vtsp_mesh_t *mesh);
static int bind_solve_heat(void* ctx, const vtsp_mesh_t *input,
			   vtsp_field_t *output);
static int bind_integrate_path(const vtsp_field_t *field,
			       const vtsp_points_t *points,
			       float* output);

int main(int argc, const char* argv[])
{
	
	uint64_t memsize = vtsp_sizeof_operational_memory();
	void *mem;
	TRY_PTR( malloc(memsize), mem, ERROR_MALLOC );

	state_t state;
	TRY( state_init(&state) );
	
	vtsp_points_t input;
	TRY( load_points(&input) );
	
	vtsp_perm_t output;
	TRY( allocate_output(&input, &output) );

	vtsp_depend_t depend;
	TRY( bind_dependencies(&depend, &state) );
	
	TRY( vtsp_solve(&input, &output, &depend, mem) );

	TRY( state_clean(&state) );
	return SUCCESS;
}

static int state_init(state_t *state)
{
	state->progress100 = 0;
	return SUCCESS;
}

static int state_clean(state_t *state)
{
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

static int bind_dependencies(vtsp_depend_t *depend, state_t *state)
{
	TRY( bind_logger(&(depend->logger)) );
	TRY( bind_drawer(&(depend->drawer)) );
	TRY( bind_reporter(&(depend->reporter), &(state->progress100)) );
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
	drawer->ctx = 0; // PENDING
	drawer->draw_state = &bind_draw_state;
	return SUCCESS;
}

static int bind_reporter(vtsp_binding_reporter_t *reporter, float *progress100)
{
	reporter->ctx = progress100;
	reporter->report_progress = &bind_report_progress;
	return SUCCESS;
}

static int bind_envelope(vtsp_binding_envelope_t *envelope)
{
	envelope->ctx = 0; // PENDING
	envelope->get_convex_envelope = &bind_get_convex_envelope;
	return SUCCESS;
}

static int bind_mesher(vtsp_binding_mesher_t *mesher)
{
	mesher->ctx = 0; // PENDING
	mesher->get_mesh = &bind_get_mesh;
	return SUCCESS;
}

static int bind_heat(vtsp_binding_heat_t *heat)
{
	heat->ctx = 0; // PENDING
	heat->solve_heat = &bind_solve_heat;
	return SUCCESS;
}

static int bind_integral(vtsp_binding_integral_t *integral)
{
	integral->ctx = 0; // PENDING
	integral->integrate_path = &bind_integrate_path;
	return SUCCESS;
}

static int bind_log(void *ctx, const char *msg) {
	FILE *fp;
	TRY_PTR(  fopen((char*) ctx, "a"), fp, ERROR );
	
	TRY_NONEG( fprintf(fp, "%s\n", msg), ERROR );

	TRY( fclose(fp) );
	return SUCCESS;
}

static int bind_draw_state(void *ctx, const vtsp_points_t *points,
			   const vtsp_mesh_t *mesh, const vtsp_field_t *field,
			   const vtsp_perm_t *path)
{
	// PENDING
	return SUCCESS;
}

static int bind_report_progress(void *ctx, float percent)
{
	float *progress100 = (float*) ctx;
	*progress100 = percent;
	
	return SUCCESS;
}

static int bind_get_convex_envelope(void* ctx, const vtsp_points_t *input,
				    vtsp_perm_t *output)
{
	// PENDING
	return SUCCESS;
}

static int bind_get_mesh(void* ctx, const vtsp_points_t *input,
			 vtsp_mesh_t *mesh)
{
	// PENDING
	return SUCCESS;
}

static int bind_solve_heat(void* ctx, const vtsp_mesh_t *input,
			   vtsp_field_t *output)
{
	// PENDING
	return SUCCESS;
}

static int bind_integrate_path(const vtsp_field_t *field,
			       const vtsp_points_t *points,
			       float* output)
{
	// PENDING
	return SUCCESS;
}
