#ifndef __VTSP_GRAPHICS__
#define __VTSP_GRAPHICS__


#include "vtsp.h"

typedef struct vtsp_graphics_s vtsp_graphics_t;

int vtsp_allocate_graphics_ctx(vtsp_graphics_t **ctx, uint32_t width, uint32_t height);
int vtsp_free_graphics_ctx(vtsp_graphics_t *ctx);
int vtsp_fill_background(vtsp_graphics_t *ctx);
int vtsp_draw_field(vtsp_graphics_t *ctx, const vtsp_mesh_t *mesh, const vtsp_field_t *field);
int vtsp_draw_mesh(vtsp_graphics_t *ctx, const vtsp_mesh_t *mesh);
int vtsp_draw_path(vtsp_graphics_t *ctx, const vtsp_points_t *points, const vtsp_perm_t *path);
int vtsp_draw_points(vtsp_graphics_t *ctx, const vtsp_points_t *points);
int vtsp_draw_save_png(vtsp_graphics_t *ctx, const char *filename);

#endif
