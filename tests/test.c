#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "delaunay_trg.h"
#include "fem_heat.h"
#include "triangular_mesh.h"
#include "tsp_io.h"

#include "try_macros.h"
#include "vtsp.h"
#include "vtsp_graphics.h"
#include "vtsp_mesh_integral.h"


enum {
	ERROR_MALLOC = 100
};

typedef struct {
	float progress100;
} state_t;

typedef struct {
	int counter;
	char *path_prefix;
	uint32_t width;
	uint32_t height;
} draw_ctx;


static int execute_vtsp(const char *input_filename);
static int log_flush(FILE* fp, const char *msg);
static int state_init(state_t *state);
static int state_clean(state_t *state);
static int input_allocate_and_load(const char* input_filename,
				   vtsp_points_t *output);
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
			   float input_temperature_vtx,
			   vtsp_field_t *output);
static int bind_integrate_path(const vtsp_field_t *field,
			       const vtsp_mesh_t *mesh,
			       uint32_t p1, uint32_t p2,
			       double* output);
static int cast_alloc_input_to_dms_solid(const vtsp_points_t *input_pts,
					 const vtsp_perm_t *input_envelope,
					 dms_solid_t *output);
static int free_dms_solid_artifacts(dms_solid_t *input);

static int opmem_dms_get_convex_envelope(const dms_points_t *input,
					 dms_perm_t *output);
static int opmem_dms_verify_solid(const dms_solid_t *input);
static int opmem_dms_get_mesh(const dms_solid_t *input,
			      const dms_extra_refine_t *refine,
			      void *output);

static int bind_should_split_trg(void *ctx, const dms_trg_ctx_t *in,
				 bool* out);
static int copy_mesh(const dms_xmesh_t *input, vtsp_mesh_t *output);

int main(int argc, const char* argv[])
{
	THROW( argc < 2, ERROR );
	
	TRY( execute_vtsp(argv[1]) );
	return SUCCESS;
}

int execute_vtsp(const char *input_filename)
{
       	vtsp_points_t input;
	TRY( log_flush(stdout, "Loading input...") );
	TRY( input_allocate_and_load(input_filename, &input) );
	
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
	TRY_NONEG( fprintf(fp, "%s\n", msg), ERROR );
	TRY_GOTO( fflush(fp), ERROR );
	return SUCCESS;
ERROR:
	return ERROR;
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

static int input_allocate_and_load(const char* input_filename,
				   vtsp_points_t *output)
{
	uint32_t npts;
	TRY_GOTO( vtsp_read_problem_npts(input_filename, &npts), ERROR_FILE );

	output->num = npts;
	size_t size = output->num * sizeof(*(output->pts));
	TRY_PTR( malloc(size), output->pts, ERROR_MALLOC );
	
	TRY_GOTO( vtsp_read_problem(input_filename, output), ERROR_READING );

	return SUCCESS;
ERROR_FILE:
	TRY( log_flush(stderr, "Error reading file") );
	return ERROR;
ERROR_MALLOC:
	TRY( log_flush(stderr, "Error allocating input") );
	return ERROR;
ERROR_READING:
	TRY( log_flush(stderr, "Error reading input") );
	free(output->pts);
	return ERROR;
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
	draw_ctx ctx;
	ctx.counter = 0;
	ctx.path_prefix = "draw";
	ctx.width = 800;
	ctx.height = 600;
	
	drawer->ctx = &ctx;
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
	heat->ctx = 0;
	heat->solve_heat = &bind_solve_heat;
	return SUCCESS;
}

static int bind_integral(vtsp_binding_integral_t *integral)
{
	integral->ctx = 0;
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
	draw_ctx *dctx = (draw_ctx*) ctx;

	char filename[100];
	int n = snprintf(filename, 100, "%s-%i.png", dctx->path_prefix, dctx->counter);
	THROW( n < 0 || n > 99, ERROR );
	
	dctx->counter += 1;

	vtsp_graphics_t *vtsp_graphics;
	TRY_GOTO( vtsp_allocate_graphics_ctx(&vtsp_graphics, dctx->width, dctx->height), ERROR );
	
	TRY_GOTO( vtsp_fill_background(vtsp_graphics), ERROR_GRAPHICS );
	TRY_GOTO( vtsp_draw_field(vtsp_graphics, mesh, field), ERROR_GRAPHICS );
	TRY_GOTO( vtsp_draw_mesh(vtsp_graphics, mesh), ERROR_GRAPHICS );
	TRY_GOTO( vtsp_draw_path(vtsp_graphics, points, path), ERROR_GRAPHICS );
	TRY_GOTO( vtsp_draw_points(vtsp_graphics, points), ERROR_GRAPHICS );
	TRY_GOTO( vtsp_draw_save_png(vtsp_graphics, filename), ERROR_GRAPHICS );
	
	TRY( vtsp_free_graphics_ctx(vtsp_graphics) );
	
	return SUCCESS;
ERROR_GRAPHICS:
	TRY( vtsp_free_graphics_ctx(vtsp_graphics) );
ERROR:
	return ERROR;
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
	
	TRY_GOTO( log_flush(stdout, "GetMesh / Verify solid... "), ERROR_BAD_INPUT );
	TRY_GOTO( opmem_dms_verify_solid(&dms_solid), ERROR_BAD_INPUT );
	
	dms_extra_refine_t dms_refiner;
	dms_refiner.ctx = 0;
	dms_refiner.max_nodes = output->nodes.n_alloc; /* Max nodes allocated for output*/
	dms_refiner.should_split_trg = &bind_should_split_trg;
	
	uint32_t memsize;
	void* dms_output;
	TRY_GOTO( log_flush(stdout, "GetMesh / Allocate output... "), ERROR_BAD_INPUT );
	TRY_GOTO( dms_get_mesh_sizeof_output(&dms_solid, &dms_refiner, &memsize), ERROR_BAD_INPUT );
	TRY_PTR( malloc(memsize), dms_output, ERROR_BAD_INPUT );
	
	TRY_GOTO( log_flush(stdout, "GetMesh / Calculate mesh... "), ERROR );
	TRY_GOTO( opmem_dms_get_mesh(&dms_solid, &dms_refiner, dms_output), ERROR );
	
	dms_xmesh_t *dms_xmesh;
	TRY_GOTO( dms_get_mesh_ref_output(dms_output, &dms_xmesh), ERROR );
	
	TRY_GOTO( log_flush(stdout, "GetMesh / Copy to output... "), ERROR );
	TRY_GOTO( copy_mesh(dms_xmesh, output), ERROR );
	
	free(dms_output);
	TRY( free_dms_solid_artifacts(&dms_solid) );
	return SUCCESS;
ERROR:
	free(dms_output);
ERROR_BAD_INPUT:
	TRY( free_dms_solid_artifacts(&dms_solid) );
	return ERROR;
}

static int bind_solve_heat(void* ctx, const vtsp_mesh_t *input,
			   float input_temperature_vtx,
			   vtsp_field_t *output)
{
	fem_cond_t fem_cond;
	fem_cond.temp.on_nodes.n = input->map_vtx.num;
	fem_cond.temp.on_nodes.index = input->map_vtx.index;
	fem_cond.temp.on_edges.n = 0;
	fem_cond.temp.on_edges.edges = 0;
	fem_cond.temp.on_edges.values = 0;
	fem_cond.flux.on_nodes.n = 0;
	fem_cond.flux.on_nodes.index = 0;
	fem_cond.flux.on_nodes.values = 0;
	fem_cond.flux.on_edges.n = 0;
	fem_cond.flux.on_edges.edges = 0;
	fem_cond.flux.on_edges.values = 0;
	
	uint32_t node_values_memsize = input->nodes.num * sizeof(*fem_cond.temp.on_nodes.values);
	float *node_values;

	TRY( log_flush(stdout, "SolveHeat / Allocating conditions memory... ") );
	TRY_PTR( malloc(node_values_memsize), node_values, ERROR_ALLOCATING_COND );

	uint32_t i;
	for (i = 0; i < input->nodes.num; i++) {
		node_values[i] = input_temperature_vtx;
	}
	fem_cond.temp.on_nodes.values = node_values;
	
	fem_mesh_t fem_mesh;
	fem_mesh.n_nodes = input->nodes.num;
	fem_mesh.nodes = (fem_node_t*) input->nodes.pts;
	fem_mesh.n_elems = input->adj.num;
	fem_mesh.elems = (fem_elem_t*) input->adj.trgs;
	
	fem_input_t fem_input;
	fem_input.diffusion = 1.0f;
	fem_input.mesh = fem_mesh;
	fem_input.cond = fem_cond;
	
	uint32_t memsize;
	void *opmem;

	TRY( log_flush(stdout, "SolveHeat / Allocating operational memory... ") );
	TRY( fem_solve_heat_sizeof_opmem(&fem_input, &memsize) );

	TRY_PTR( malloc(memsize), opmem, ERROR_ALLOCATING );
	
	TRY_GOTO( log_flush(stdout, "SolveHeat / Solving... "), ERROR );
	TRY_GOTO( fem_solve_heat(&fem_input, output->values, opmem), ERROR );

	free(opmem);
	return SUCCESS;
ERROR:
	free(opmem);
ERROR_ALLOCATING:
	free(node_values);
ERROR_ALLOCATING_COND:
	return ERROR;
}

static int bind_integrate_path(const vtsp_field_t *field,
			       const vtsp_mesh_t *mesh,
			       uint32_t p1, uint32_t p2,
			       double* output)
{
	uint32_t n1 = mesh->map_vtx.index[p1];
	uint32_t n2 = mesh->map_vtx.index[p2];
	TRY( vtsp_integrate_path(mesh, field, n1, n2, output) );
	
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

	uint32_t i;
	for (i = 0; i < nsgm; i++) {
		output->sgm.data[i].p1 = input_envelope->index[i];
		output->sgm.data[i].p2 = input_envelope->index[(i+1) % nsgm];
	}
	
	return SUCCESS;
	
ERROR_MALLOC:
	return ERROR_MALLOC;
}

static int free_dms_solid_artifacts(dms_solid_t *input)
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
			      void *output)
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

static int copy_mesh(const dms_xmesh_t *input, vtsp_mesh_t *output)
{
	/* Copy nodes */
	THROW( input->mesh.nodes.num > output->nodes.n_alloc, ERROR );
	output->nodes.num = input->mesh.nodes.num;

	uint32_t i = 0;
	for (i = 0; i < output->nodes.num; i++) {
		output->nodes.pts[i].x = input->mesh.nodes.data[i].x;
		output->nodes.pts[i].y = input->mesh.nodes.data[i].y;
	}

	/* Copy triangles */
	THROW( input->mesh.trgs.num > output->adj.n_alloc, ERROR );
	output->adj.num = input->mesh.trgs.num;
	for (i = 0; i < output->adj.num; i++ ) {
		output->adj.trgs[i].n1 = input->mesh.trgs.data[i].n1;
		output->adj.trgs[i].n2 = input->mesh.trgs.data[i].n2;
		output->adj.trgs[i].n3 = input->mesh.trgs.data[i].n3;
	}

	/* Copy vtx map */
	THROW( input->map_vtx.num > output->map_vtx.n_alloc, ERROR );
	output->map_vtx.num = input->map_vtx.num;
	for (i = 0; i < output->map_vtx.num; i++) {
		output->map_vtx.index[i] = input->map_vtx.index[i];
	}
	
	return SUCCESS;
}
