#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "delaunay_trg.h"
#include "triangular_mesh.h"

#include "try_macros.h"
#include "vtsp.h"


#define MAX_MESH_VTX 2000

enum {
	ERROR_MALLOC = 100
};

typedef struct {
	float progress100;
} state_t;

static int log_flush(FILE* fp, const char *msg);
static int state_init(state_t *state);
static int state_clean(state_t *state);
static int input_allocate_and_load(vtsp_points_t *input);
static int input_free(vtsp_points_t *input);
static int output_allocate(const vtsp_points_t *input, vtsp_perm_t *output);
static int output_free(vtsp_perm_t *output);
static int output_save(vtsp_perm_t *output);

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
static int bind_get_mesh(void* ctx, const vtsp_points_t *input_pts,
			 const vtsp_perm_t *input_envelope,
			 vtsp_mesh_t *output);
static int bind_solve_heat(void* ctx, const vtsp_mesh_t *input,
			   vtsp_field_t *output);
static int bind_integrate_path(const vtsp_field_t *field,
			       const vtsp_points_t *points,
			       float* output);
static int cast_alloc_input_to_dms_solid(const vtsp_points_t *input_pts,
					 const vtsp_perm_t *input_envelope,
					 dms_solid_t *output);
static int free_dms_solid(dms_solid_t *input);

static int opmem_dms_get_convex_envelope(const dms_points_t *input,
					 dms_perm_t *output);
static int opmem_dms_verify_solid(const dms_solid_t *input);
static int opmem_dms_get_mesh(const dms_solid_t *input,
			      const dms_extra_refine_t *refine,
			      dms_xmesh_t *output);

static int bind_should_split_trg(void *ctx, const dms_trg_ctx_t *in,
				 bool* out);

int main(int argc, const char* argv[])
{
       	vtsp_points_t input;
	TRY( log_flush(stdout, "Loading input...") );
	TRY( input_allocate_and_load(&input) );
	
	vtsp_perm_t output;
	TRY_GOTO( log_flush(stdout, "Allocating output... "), ERROR_OUTPUT );
	TRY_GOTO( output_allocate(&input, &output), ERROR_OUTPUT );

	uint32_t memsize;
	void *opmem;
	TRY_GOTO( log_flush(stdout, "Allocating operational memory... "), ERROR_MALLOC );
	TRY_GOTO( vtsp_solve_sizeof_opmem(&input, &memsize), ERROR_MALLOC );
	TRY_PTR( malloc(memsize), opmem, ERROR_MALLOC );
	
	state_t state;
	TRY_GOTO( log_flush(stdout, "Initializing state... "), ERROR_STATE );
	TRY_GOTO( state_init(&state), ERROR_STATE );

	vtsp_depend_t depend;
	TRY_GOTO( log_flush(stdout, "Binding dependencies... "), ERROR );
	TRY_GOTO( bind_dependencies(&depend, &state), ERROR );
	
	TRY_GOTO( log_flush(stdout, "Solving TSP... "), ERROR );
	TRY_GOTO( vtsp_solve(&input, &output, &depend, opmem), ERROR );


	TRY_GOTO( log_flush(stdout, "Saving output... "), ERROR );
        TRY_GOTO( output_save(&output), ERROR );
	
	TRY( state_clean(&state) );
	free(opmem);
	TRY( output_free(&output) );
	TRY( input_free(&input) );
	return SUCCESS;
ERROR:
	TRY( state_clean(&state) );
ERROR_STATE:
	free(opmem);
ERROR_MALLOC:
	TRY( output_free(&output) );
ERROR_OUTPUT:
	TRY( input_free(&input) );
	TRY( log_flush(stderr, "Error solving TSP") );
	return ERROR;
}

static int log_flush(FILE* fp, const char *msg)
{
	fprintf(fp, "%s\n", msg);
	fflush(fp);
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

static int input_allocate_and_load(vtsp_points_t *input)
{
	// PENDING: Load from file
	input->num = 10;
	size_t size = input->num * sizeof(*(input->pts));
	TRY_PTR( malloc(size), input->pts, ERROR_MALLOC );

	int i;
	for (i = 0; i < input->num; i++) {
		input->pts[i].x = i;
		input->pts[i].y = i*i;
	}
	
	return SUCCESS;
ERROR_MALLOC:
	return ERROR_MALLOC;
}

static int input_free(vtsp_points_t *input)
{
	free(input->pts);
	return SUCCESS;
}

static int output_allocate(const vtsp_points_t *input, vtsp_perm_t *output)
{
	output->num = input->num;
	
	size_t size = output->num * sizeof(*(output->index));
	TRY_PTR( malloc(size), output->index, ERROR_MALLOC );

	return SUCCESS;
ERROR_MALLOC:
	return ERROR_MALLOC;
}

static int output_free(vtsp_perm_t *output)
{
	free(output->index);
	return SUCCESS;
}

static int output_save(vtsp_perm_t *output)
{	
	FILE *fp;
	TRY_PTR(  fopen("output.txt", "w"), fp, ERROR );
	
	TRY_NONEG( fprintf(fp, "%u\n", output->num), ERROR );
	int i;
	for (i = 0; i < output->num; i++) {
		TRY_NONEG( fprintf(fp, "%u ", output->index[i]), ERROR );		
	}
	TRY_NONEG( fprintf(fp, "\n"), ERROR );

	TRY( fclose(fp) );
	return SUCCESS;
ERROR:
	return ERROR;
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
	envelope->ctx = 0;
	envelope->get_convex_envelope = &bind_get_convex_envelope;
	return SUCCESS;
}

static int bind_mesher(vtsp_binding_mesher_t *mesher)
{
	mesher->ctx = 0;
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
ERROR:
	return ERROR;
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
	dms_points_t dms_input;
	dms_input.num = input->num;
	dms_input.data = (dms_point_t*) input->pts;

	dms_perm_t dms_output;
	dms_output.num = output->num;
	dms_output.index = output->index;
	
	TRY( opmem_dms_get_convex_envelope(&dms_input, &dms_output) );
	
	return SUCCESS;
}


static int bind_get_mesh(void* ctx, const vtsp_points_t *input_pts,
			 const vtsp_perm_t *input_envelope,
			 vtsp_mesh_t *output)
{
	dms_solid_t dms_solid;
	TRY( cast_alloc_input_to_dms_solid(input_pts, input_envelope, &dms_solid) );
	
	TRY_GOTO( opmem_dms_verify_solid(&dms_solid), ERROR_SOLID );
	
	dms_extra_refine_t dms_refiner;
	dms_refiner.ctx = 0;
	dms_refiner.max_vtx = MAX_MESH_VTX;
	dms_refiner.should_split_trg = &bind_should_split_trg;
	
	dms_xmesh_t dms_xmesh;
	// PENDING CAST OUTPUT TO DMS OUTPUT

	TRY_GOTO( opmem_dms_get_mesh(&dms_solid, &dms_refiner, &dms_xmesh),
		  ERROR_MESH );

	// PENDING CAST OUTPUT TO VTSP MESH
	free_dms_solid(&dms_solid);
	return SUCCESS;
ERROR_MESH:
ERROR_SOLID:
	free_dms_solid(&dms_solid);
	return ERROR;
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

static int cast_alloc_input_to_dms_solid(const vtsp_points_t *input_pts,
					 const vtsp_perm_t *input_envelope,
					 dms_solid_t *output)
{
	output->vtx.num = input_pts->num;
	output->vtx.data = (dms_point_t*) input_pts->pts;
	output->holes.num = 0;
	output->holes.data = NULL;

	uint32_t nsgm = input_envelope->num;
	output->sgm.num = nsgm;
	TRY_PTR( malloc(nsgm * sizeof(*(output->sgm.data))), output->sgm.data,
		 ERROR_MALLOC ); 
	return SUCCESS;
ERROR_MALLOC:
	return ERROR_MALLOC;
}

static int free_dms_solid(dms_solid_t *input)
{
	free(input->sgm.data);
	return SUCCESS;
}

static int opmem_dms_get_convex_envelope(const dms_points_t *input,
					 dms_perm_t *output)
{
	uint32_t memsize;
	void *opmem;
	TRY( log_flush(stdout, "Getting convex envelope...") );
	TRY( dms_get_convex_envelope_sizeof_opmem(input, &memsize) );
	TRY_PTR( malloc(memsize), opmem, ERROR_MALLOC );

	TRY_GOTO( dms_get_convex_envelope(input, output, opmem), ERROR);

	free(opmem);
	return SUCCESS;
ERROR:
	free(opmem);
ERROR_MALLOC:
	return ERROR;
}

static int opmem_dms_verify_solid(const dms_solid_t *input)
{
	uint32_t memsize;
	void *opmem;
	TRY( log_flush(stdout, "Verifying solid...") );
	TRY( dms_verify_solid_sizeof_opmem(input, &memsize) );
	TRY_PTR( malloc(memsize), opmem, ERROR_MALLOC );

	char error_msg[100];
	TRY_GOTO( dms_verify_solid(input, error_msg, opmem), ERROR );

	free(opmem);
	
	TRY( log_flush(stdout, error_msg) );
	return SUCCESS;
ERROR:
	free(opmem);
ERROR_MALLOC:
	return ERROR;
}

static int opmem_dms_get_mesh(const dms_solid_t *input,
			      const dms_extra_refine_t *refine,
			      dms_xmesh_t *output)
{
	uint32_t memsize;
	void *opmem;
	TRY( log_flush(stdout, "Meshing solid with DMS...") );
	TRY( dms_get_mesh_sizeof_opmem(input, refine, &memsize) );
	TRY_PTR( malloc(memsize), opmem, ERROR_MALLOC );

	TRY_GOTO( dms_get_mesh(input, refine, output, opmem), ERROR);
	
	free(opmem);
	return SUCCESS;
ERROR:
	free(opmem);
ERROR_MALLOC:
	return ERROR;
}

static int bind_should_split_trg(void *ctx, const dms_trg_ctx_t *in, bool* out)
{
	*out = false;
	return SUCCESS;
}
