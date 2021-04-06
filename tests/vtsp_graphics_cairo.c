#include <stdlib.h>
#include <stdint.h>
#include <cairo.h>
#include <math.h>


#include "graphics.h"

#include "try_macros.h"
#include "vtsp.h"

struct vtsp_graphics_s {
	cairo_t *cr;
	cairo_surface_t *surface;
	uint32_t w, h;
};

int vtsp_allocate_graphics_ctx(vtsp_graphics_t **ctx, uint32_t width, uint32_t height) {
	TRY_PTR( malloc(sizeof(**ctx)), *ctx, ERROR );
	
	TRY_PTR( cairo_image_surface_create_for_data(image, CAIRO_FORMAT_ARGB32, width, height, STRIDE), (*ctx)->surface, ERROR );
	TRY_PTR( cairo_create(surface), (*ctx)->cr, ERROR_CR );

	(*ctx)->w = width;
	(*ctx)->h = height;
	
	return SUCCESS;
ERROR_CR:
	cairo_surface_destroy(ctx->surface);
ERROR:
	return ERROR;
}


int vtsp_free_graphics_ctx(vtsp_graphics_t *ctx) {

	cairo_destroy(ctx->cr);
	cairo_surface_destroy(ctx->surface);
	
	free(ctx);
	return SUCCESS;
}

int vtsp_fill_background(vtsp_graphics_t *ctx) {
	cairo_rectangle(ctx->cr, 0, 0, ctx->w, ctx->h);
	cairo_set_source_rgb(ctx->cr, 0.7, 0.7, 0.7);
	cairo_fill(ctx->cr);
	return SUCCESS;
}

int vtsp_draw_field(vtsp_graphics_t *ctx, const vtsp_mesh_t *mesh, const vtsp_field_t *field) {
	// PENDING
	return SUCCESS;
}

int vtsp_draw_mesh(vtsp_graphics_t *ctx, const vtsp_mesh_t *mesh) {
	// PENDING
	return SUCCESS;
}

int vtsp_draw_path(vtsp_graphics_t *ctx, const vtsp_points_t *points, const vtsp_perm_t *path) {
	if (path->num == 0) {
		return SUCCESS;
	}

	vtsp_point_t p = points->pts[0];
	double x = p.x;
	double y = p.y;
	cairo_move_to(ctx->cr, x, y);
	
	uint32_t i;
	for (i = 1; i < path->num; i++) {
		p = points->pts[i];
		x = p.x;
		y = p.y;
		cairo_line_to(ctx->cr, x, y);
	}

	cairo_close_path(ctx->cr);
	
	cairo_set_source_rgb(ctx->cr, 0.0, 0.0, 0.0);
	cairo_set_line_width(cr, 2);
	cairo_stroke(ctx->cr);

	return SUCCESS;
}

int vtsp_draw_points(vtsp_graphics_t *ctx, const vtsp_points_t *points) {
	double pi2 = 2.0 * M_PI;
	double r = 3.0;

	uint32_t i;
	cairo_set_source_rgb(ctx->cr, 0.0, 0.0, 0.0);
	for (i = 1; i < path->num; i++) {
		p = points->pts[i];
		x = p.x;
		y = p.y;
		cairo_arc(ctx->cr, x, y, r, 0.0, pi2);
		cairo_fill(ctx->cr);
	}

	return SUCCESS;
}

int vtsp_draw_save_png(vtsp_graphics_t *ctx, const char *filename) {
	cairo_surface_write_to_png(ctx->surface, filename);
}
