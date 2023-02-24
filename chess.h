#pragma once

#include <SDL2/SDL.h>
#include "types.h"

/* ============================ */
/*           GLOBALS            */
/* ============================ */

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800

extern SDL_Window* sdl_window;
extern SDL_GLContext sdl_gl_context;
extern bool8 window_open;

/* ============================ */
/*        SDL LIBRARY           */
/* ============================ */

void sdl_assert_msg(bool8 cond, const char* errmsg);

void sdl_setup_window(const char* window_title);

/* ============================ */
/*   APPLICATION STRUCTURE API  */
/* ============================ */

void application_loop();

void application_terminate();

/* ============================ */
/*      OPENGL SHADER API       */
/* ============================ */

typedef struct {
    u32 id;
} opengl_shader;

opengl_shader opengl_shader_create(const char* vertex_path, const char* fragment_path);

void opengl_shader_bind(opengl_shader shader);

void opengl_shader_unbind(); 

void opengl_shader_delete(opengl_shader shader);

void opengl_shader_upload_int(opengl_shader shader, int i, const char* name);

void opengl_shader_upload_float(opengl_shader shader, float f, const char* name);

void opengl_shader_upload_vec2(opengl_shader shader, vec2 v, const char* name);

void opengl_shader_upload_vec3(opengl_shader shader, vec3 v, const char* name);

void opengl_shader_upload_vec4(opengl_shader shader, vec4 v, const char* name);

void opengl_shader_upload_mat4(opengl_shader shader, mat4 m, const char* name);


/* ============================ */
/*     OPENGL TEXTURE API       */
/* ============================ */

typedef struct {
    u32 id;
    u32 width, height;
} opengl_texture;

opengl_texture opengl_texture_create(const char* path);

void opengl_texture_bind(opengl_texture texture);

void opengl_texture_unbind();

void opengl_texture_bind_unit(opengl_texture texture, u32 slot);

void opengl_texture_delete(opengl_texture texture);


typedef struct {
    opengl_texture texture;
    vec2 coords, cell_size, sprite_scale;
} subtexture;

typedef struct {
    vec2 min, max;
} subtexture_coords;

subtexture subtexture_create(opengl_texture texture, vec2 coords, vec2 cell_size, vec2 sprite_scale);

subtexture_coords subtexture_get_texcoords(subtexture texture);

/* ============================ */
/*         RENDERER API         */
/* ============================ */

void init_quad_renderer();

void terminate_quad_renderer();

void render_quad(vec2 uv,vec4 color, vec2 pos, vec2 scale);

void render_quad_color(vec4 color, vec2 pos, vec2 scale);

/* ============================ */
/*        CHESS GAME API        */
/* ============================ */

void render_chess_board_bg();

void init_chess_board(); 

void destroy_chess_board();

void add_chess_piece_to_board(vec2 pos, vec2 uv);

void move_chess_piece_on_board(vec2 src_pos, vec2 dst_pos);

void render_chess_pieces_on_board();
