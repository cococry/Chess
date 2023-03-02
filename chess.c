#include "chess.h"
#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
#include <stb_image.h>
#include <string.h>

/* ============================ */
/*           GLOBALS            */
/* ============================ */

SDL_Window* sdl_window;
SDL_GLContext sdl_gl_context;
bool8 window_open;

typedef struct {
    bool8 white_turn;
    chess_piece selected_chess_piece;
} game_state;

static game_state s_game_state;

typedef struct {
    chess_piece* pieces;
    u32 piece_count;
} chess_board;

static chess_board board_data;

#define BOARD_X_SIZE 8
#define BOARD_Y_SIZE 8

static bool8 selected_any_chess_piece() { 
    return s_game_state.selected_chess_piece.board_pos.x != -1.0f && s_game_state.selected_chess_piece.board_pos.y != -1.0f;
}
static bool8 piece_is_playing_color(chess_piece piece) {
    return ((s_game_state.white_turn && piece.is_white) ||
                    (!s_game_state.white_turn && !piece.is_white));
}
static bool8 is_piece_on_board_pos(vec2 board_pos) {
    return get_chess_piece_by_board_pos(board_pos).type != chess_piece_type_none;
}

/* ============================ */
/*        SDL LIBRARY           */
/* ============================ */

void sdl_assert_msg(bool8 cond, const char* errmsg) {
    if (!cond) {
        printf("%s | SDL Error: %s\n", errmsg, SDL_GetError());
        exit(exit_failure);
    }
}

void sdl_setup_window(const char* window_title) {
    sdl_assert_msg(SDL_Init(SDL_INIT_EVERYTHING) == 0, "Failed to initialize SDL");
    sdl_assert_msg(SDL_GL_LoadLibrary(NULL) == 0, "Failed to load default GL library");

    sdl_assert_msg(SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1) == 0, "Failed to set GL accelerated visual");
    sdl_assert_msg(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4) == 0, "Failed to set GL Major version");
    sdl_assert_msg(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6) == 0, "Failed to set GL Minor version");

    sdl_assert_msg(SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1) == 0, "Failed to enable double buffering");
    sdl_assert_msg(SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24) == 0, "Failed to set depth size");

    sdl_window = SDL_CreateWindow(
        window_title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    sdl_assert_msg(sdl_window != NULL, "Failed to initialize SDL window");

    sdl_gl_context = SDL_GL_CreateContext(sdl_window);
    sdl_assert_msg(sdl_gl_context != NULL, "Failed to create GL context");

    gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

    printf("OpenGL vendor: %s\n", glGetString(GL_VENDOR));
    printf("OpenGL renderer: %s\n", glGetString(GL_RENDERER));
    printf("OpenGL version: %s\n", glGetString(GL_VERSION));

    sdl_assert_msg(SDL_GL_SetSwapInterval(1) == 0, "Swap Interval not supported.");

    i32 width, height;
    SDL_GetWindowSize(sdl_window, &width, &height);
    glViewport(0, 0, width, height);
}

/* ============================ */
/* APPLICATION STRUCTURE API*/
/* ============================ */

void application_loop() {
    window_open = true;
    SDL_Event ev;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    init_quad_renderer();
    init_chess_board();
    chess_board_default_placement();

    s_game_state.selected_chess_piece.board_pos = (vec2){-1.0f, -1.0f};
    s_game_state.white_turn = true; 
    
    bool8 should_reset_game = false;

    while (window_open) {
        glClear(GL_COLOR_BUFFER_BIT);    
        glClearColor(0.2f, 0.3f, 0.8f, 1.0f);
        render_chess_board_bg();
        render_chess_pieces_on_board();
        vec2 available_moves[96];
        memset(available_moves, -1.0f, sizeof(available_moves));
        u32 available_moves_count = 0;

        if(is_king_in_check(!s_game_state.white_turn)) {
                render_quad_on_chess_board((vec4){0.8f, 0.2f, 0.3f, 1.0f}, get_king_position(!s_game_state.white_turn), 0.5f);
        } 
        if(is_king_in_check(s_game_state.white_turn)) {
            render_quad_on_chess_board((vec4){0.8f, 0.2f, 0.3f, 1.0f}, get_king_position(s_game_state.white_turn), 0.5f);
        }

        if (selected_any_chess_piece()) {
            render_quad_on_chess_board((vec4){0.2f, 0.3f, 0.8f, 0.4f}, s_game_state.selected_chess_piece.board_pos, 1.0f);

            available_moves_count = get_available_moves_from_chess_piece(s_game_state.selected_chess_piece, available_moves);
            if (piece_is_playing_color(s_game_state.selected_chess_piece)) {
                for (u32 i = 0; i < available_moves_count; i++) {
                    if(is_king_in_check_after_move(s_game_state.selected_chess_piece.board_pos, available_moves[i], !s_game_state.white_turn)) {
                        render_quad_on_chess_board((vec4){0.8f, 0.2f, 0.3f, 1.0f}, available_moves[i], 0.5f);
                        available_moves[i] = (vec2){-1.0f, -1.0f};
                    } else {
                        render_quad_on_chess_board((vec4){0.2f, 0.8f, 0.3f, 1.0f}, available_moves[i], 0.5f); 
                    }
                }
            }
        }

        if(is_king_in_check(!s_game_state.white_turn) || is_king_in_check(s_game_state.white_turn)) {
            bool8 has_available_moves = false;
            for(u32 i = 0; i < available_moves_count; i++) {
                if(available_moves[i].x != -1.0f && available_moves[i].y != -1.0f) {
                    has_available_moves = true;
                }
            }
            if(!has_available_moves) {
                if(!should_reset_game)
                    printf("%s won the game!\n", !s_game_state.white_turn ? "White" : "Black");
                should_reset_game = true;
            }
        }

        SDL_GL_SwapWindow(sdl_window);

        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) {
                window_open = false;
            }
            if (ev.type == SDL_MOUSEBUTTONDOWN) {
                if (ev.button.button == SDL_BUTTON_LEFT) {
                    if(should_reset_game) {
                        chess_board_default_placement();
                        s_game_state.selected_chess_piece.board_pos = (vec2){-1.0f, -1.0f};
                        s_game_state.white_turn = true;
                        should_reset_game = false;
                    } else {
                        i32 x, y;
                        SDL_GetMouseState(&x, &y);  
                        int x_grid = (int)(x / (WINDOW_WIDTH / BOARD_X_SIZE));
                        int y_grid = (int)(y / (WINDOW_HEIGHT / BOARD_Y_SIZE));
                    
                        if (is_piece_on_board_pos((vec2){x_grid, y_grid}) && 
                            piece_is_playing_color(get_chess_piece_by_board_pos((vec2){x_grid, y_grid}))) {
                            s_game_state.selected_chess_piece = get_chess_piece_by_board_pos((vec2){x_grid, y_grid});
                        } else {
                            bool8 can_delesect = true;
                            for (u32 i = 0; i < available_moves_count; i++) {
                                if (available_moves[i].x == x_grid && available_moves[i].y == y_grid) {
                                    can_delesect = false;
                                    break;
                                }
                            }
                            if (can_delesect)
                                s_game_state.selected_chess_piece.board_pos = (vec2){-1.0f, -1.0f};
                        }
                        for (u32 i = 0; i < available_moves_count; i++) {
                            if(available_moves[i].x == -1.0f || available_moves[i].y == -1.0f) continue;
                            if (available_moves[i].x == x_grid && available_moves[i].y == y_grid) {
                                if (piece_is_playing_color(s_game_state.selected_chess_piece)) {
                                    s_game_state.white_turn = !s_game_state.white_turn;
                                    vec2 selected_move = available_moves[i];
                                    if (is_piece_on_board_pos(selected_move)) {
                                        remove_chess_piece_from_board(selected_move);
                                    }      
                                    move_chess_piece_on_board(s_game_state.selected_chess_piece.board_pos, selected_move);
                                    s_game_state.selected_chess_piece.board_pos = selected_move;
                                    if (s_game_state.selected_chess_piece.type == chess_piece_type_pawn) {
                                    for (u32 i = 0; i < board_data.piece_count; i++) {
                                        if (board_data.pieces[i].board_pos.x == s_game_state.selected_chess_piece.board_pos.x &&
                                            board_data.pieces[i].board_pos.y == s_game_state.selected_chess_piece.board_pos.y) {
                                            board_data.pieces[i].pawn_moved = true;
                                            }
                                        }
                                    }                                
                                }
                                s_game_state.selected_chess_piece.board_pos = (vec2){-1.0f, -1.0f};
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    destroy_chess_board();
    terminate_quad_renderer();
}
void application_terminate() {
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
}

/* ============================ */
/*      OPENGL SHADER API       */
/* ============================ */

static const char* read_shader_file(const char* path) {
    char* buffer = NULL;
    long length;
    FILE* f = fopen(path, "rb");

    if (f) {
        fseek(f, 0, SEEK_END);
        length = ftell(f);
        fseek(f, 0, SEEK_SET);
        buffer = malloc(length);
        if (buffer) {
            fread(buffer, 1, length, f);
        }
        fclose(f);
        buffer[length - 1] = '\0';
    }
    return buffer;
}
static u32 compile_opengl_shader(const char* path, GLenum type) {
    int compilation_success;
    char info_log[512];

    u32 shader = glCreateShader(type);

    const char* shader_source = read_shader_file(path);
    glShaderSource(shader, 1, &shader_source, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &compilation_success);

    const char* shader_name = (type == GL_VERTEX_SHADER) ? "vertex" : "fragment";
    if (!compilation_success) {
        printf("Failed to compile %s shader.\n", shader_name);
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        printf("%s\n", info_log);
    } else {
        printf("Successfully compiled %s shader '%s'.\n", shader_name, path);
    }
    return shader;
}

static void link_opengl_shader_program(opengl_shader* program, u32 vertex_shader, u32 fragment_shader) {
    int link_success;
    char info_log[512];

    program->id = glCreateProgram();

    glAttachShader(program->id, vertex_shader);
    glAttachShader(program->id, fragment_shader);

    glLinkProgram(program->id);

    glGetProgramiv(program->id, GL_LINK_STATUS, &link_success);

    if (!link_success) {
        printf("Failed to link shader program.\n");
        glGetProgramInfoLog(program->id, 512, NULL, info_log);
        printf("%s\n", info_log);
    } else {
        printf("Successfully linked shader program (ID: %i)\n", program->id);
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

opengl_shader opengl_shader_create(const char* vertex_path, const char* fragment_path) {
    opengl_shader program;

    u32 vertex_shader = compile_opengl_shader(vertex_path, GL_VERTEX_SHADER);
    u32 fragment_shader = compile_opengl_shader(fragment_path, GL_FRAGMENT_SHADER);

    link_opengl_shader_program(&program, vertex_shader, fragment_shader);

    return program;
}

void opengl_shader_bind(opengl_shader shader) {
    glUseProgram(shader.id);
}

void opengl_shader_unbind() {
    glUseProgram(0);
}

void opengl_shader_delete(opengl_shader shader) {
    glDeleteShader(shader.id);
}

void opengl_shader_upload_int(opengl_shader shader, int i, const char* name) {
    glUniform1i(glGetUniformLocation(shader.id, name), i);
}

void opengl_shader_upload_float(opengl_shader shader, float f, const char* name) {
    glUniform1f(glGetUniformLocation(shader.id, name), f);
}

void opengl_shader_upload_vec2(opengl_shader shader, vec2 v, const char* name) {
    glUniform2f(glGetUniformLocation(shader.id, name), v.x, v.y);
}

void opengl_shader_upload_vec3(opengl_shader shader, vec3 v, const char* name) {
    glUniform3f(glGetUniformLocation(shader.id, name), v.x, v.y, v.z);
}

void opengl_shader_upload_vec4(opengl_shader shader, vec4 v, const char* name) {
    glUniform4f(glGetUniformLocation(shader.id, name), v.x, v.y, v.z, v.w);
}

void opengl_shader_upload_mat4(opengl_shader shader, mat4 m, const char* name) {
    glUniformMatrix4fv(glGetUniformLocation(shader.id, name), 1, false, (float*)&m);
}

/* ============================ */
/*     OPENGL TEXTURE API       */
/* ============================ */

opengl_texture opengl_texture_create(const char* path) {
    opengl_texture ret;

    stbi_set_flip_vertically_on_load(false);

    i32 width, height, number_of_channels;
    u8* data = stbi_load(path, &width, &height, &number_of_channels, 0);

    if (!data) {
        printf("Failed to load texture '%s'.\n", path);
        return ret;
    }
    ret.width = width;
    ret.height = height;

    glGenTextures(1, &ret.id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ret.id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    return ret;
}

void opengl_texture_bind(opengl_texture texture) {
    glBindTexture(GL_TEXTURE_2D, texture.id);
}

void opengl_texture_unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}

void opengl_texture_bind_unit(opengl_texture texture, u32 slot) {
    glBindTextureUnit(slot, texture.id);
}

void opengl_texture_delete(opengl_texture texture) {
    glDeleteTextures(1, &texture.id);
}

subtexture subtexture_create(opengl_texture texture, vec2 coords, vec2 cell_size, vec2 sprite_scale) {
    subtexture ret;
    ret.texture = texture;
    ret.coords = coords;
    ret.cell_size = cell_size;
    ret.sprite_scale = sprite_scale;
    return ret;
}

subtexture_coords subtexture_get_texcoords(subtexture texture) {
    vec2 min = (vec2){(float)((texture.coords.x * texture.cell_size.x) / texture.texture.width),
                      (float)(texture.coords.y * texture.cell_size.y) / texture.texture.height};
    vec2 max = (vec2){(float)((texture.coords.x + texture.sprite_scale.x) * texture.cell_size.x) / texture.texture.width,
                      (float)(((texture.coords.y + texture.sprite_scale.y) * texture.cell_size.y) / texture.texture.height)};

    subtexture_coords coords;
    coords.min = min;
    coords.max = max;
    return coords;
}

/* ============================ */
/*         RENDERER API         */
/* ============================ */

typedef struct {
    u32 vao, vbo, ibo;
    opengl_shader shader;
    opengl_texture spritesheet;
} render_data;

static render_data r_data;

void init_quad_renderer() {
    glCreateVertexArrays(1, &r_data.vao);
    glBindVertexArray(r_data.vao);

    r_data.spritesheet = opengl_texture_create("spritesheet.png");

    u32 indices[] = {0, 1, 2, 2, 3, 0};

    glCreateBuffers(1, &r_data.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, r_data.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 5 * 6, NULL, GL_DYNAMIC_DRAW);

    glCreateBuffers(1, &r_data.ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r_data.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, NULL);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(intptr_t)(sizeof(float) * 3));

    r_data.shader = opengl_shader_create("vert.glsl", "frag.glsl");
    opengl_shader_bind(r_data.shader);
}

void terminate_quad_renderer() {
    opengl_shader_delete(r_data.shader);
    opengl_texture_delete(r_data.spritesheet);
}
void render_quad(vec2 uv, vec4 color, vec2 pos, vec2 scale) {
    opengl_texture_bind(r_data.spritesheet);
    opengl_shader_bind(r_data.shader);
    opengl_shader_upload_int(r_data.shader, 0, "u_texture");
    opengl_shader_upload_mat4(r_data.shader, mat4_orthographic(0.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.0f), "u_proj");
    opengl_shader_upload_mat4(r_data.shader, mat4_mul(mat4_translate(mat4_identity(), (vec3){pos.x, pos.y, 0.0f}), mat4_scale(mat4_identity(), (vec3){scale.x, scale.y, 0.0f})), "u_model");
    opengl_shader_upload_int(r_data.shader, true, "u_using_textures");
    opengl_shader_upload_vec4(r_data.shader, color, "u_color");
    subtexture subtex = subtexture_create(r_data.spritesheet,
                                          uv, (vec2){64.0f, 64.0f}, (vec2){1.0f, 1.0f});

    subtexture_coords coords = subtexture_get_texcoords(subtex);
    float vertices[] = {
        -0.5f, -0.5f, 0.0f, coords.min.x, coords.min.y,
        -0.5f, 0.5f, 0.0f, coords.min.x, coords.max.y,
        0.5f, 0.5f, 0.0f, coords.max.x, coords.max.y,
        0.5f, -0.5f, 0.0f, coords.max.x, coords.min.y};
    glBindVertexArray(r_data.vao);
    glBindBuffer(GL_ARRAY_BUFFER, r_data.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

void render_quad_color(vec4 color, vec2 pos, vec2 scale) {
    opengl_shader_bind(r_data.shader);
    opengl_shader_upload_int(r_data.shader, 0, "u_texture");
    opengl_shader_upload_mat4(r_data.shader, mat4_orthographic(0.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.0f), "u_proj");
    opengl_shader_upload_mat4(r_data.shader, mat4_mul(mat4_translate(mat4_identity(), (vec3){pos.x, pos.y, 0.0f}), mat4_scale(mat4_identity(), (vec3){scale.x, scale.y, 0.0f})), "u_model");
    opengl_shader_upload_int(r_data.shader, false, "u_using_textures");
    opengl_shader_upload_vec4(r_data.shader, color, "u_color");
    float vertices[] = {
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
        0.5f, -0.5f, 0.0f, 1.0f, 0.0f};
    glBindVertexArray(r_data.vao);
    glBindBuffer(GL_ARRAY_BUFFER, r_data.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

/* ============================ */
/*        CHESS GAME API        */
/* ============================ */

void render_chess_board_bg() {
    for (u32 y = 0; y < BOARD_Y_SIZE; y++) {
        for (u32 x = 0; x < BOARD_X_SIZE; x++) {
            bool8 cond = (y % 2 != 0) ? x % 2 == 0 : x % 2 != 0;
            if (cond) {
                render_quad_color((vec4){242 / 255.0f, 225 / 255.0f, 172 / 255.0f, 1.0f},
                                  (vec2){50 + WINDOW_HEIGHT / BOARD_X_SIZE * x, 50.0f + WINDOW_WIDTH / BOARD_Y_SIZE * y}, (vec2){WINDOW_HEIGHT / BOARD_X_SIZE, WINDOW_WIDTH / BOARD_Y_SIZE});
            } else {
                render_quad_color((vec4){13 / 255.0f, 115 / 255.0f, 40 / 255.0f, 1.0f},
                                  (vec2){50 + WINDOW_HEIGHT / BOARD_X_SIZE * x, 50.0f + WINDOW_WIDTH / BOARD_Y_SIZE * y}, (vec2){WINDOW_HEIGHT / BOARD_X_SIZE, WINDOW_WIDTH / BOARD_Y_SIZE});
            }
        }
    }
}

void init_chess_board() {
    board_data.pieces = malloc(sizeof(chess_piece) * 64);
    board_data.piece_count = 0;
}

void chess_board_default_placement() {
    free(board_data.pieces); 
    board_data.pieces = malloc(sizeof(chess_piece) * 64);
    board_data.piece_count = 0;

    add_chess_piece_to_board((vec2){0.0f, 0.0f}, chess_piece_type_rook, false);
    add_chess_piece_to_board((vec2){1.0f, 0.0f}, chess_piece_type_knight, false);
    add_chess_piece_to_board((vec2){2.0f, 0.0f}, chess_piece_type_bishop, false);
    add_chess_piece_to_board((vec2){3.0f, 0.0f}, chess_piece_type_queen, false);
    add_chess_piece_to_board((vec2){4.0f, 0.0f}, chess_piece_type_king, false);
    add_chess_piece_to_board((vec2){5.0f, 0.0f}, chess_piece_type_bishop, false);
    add_chess_piece_to_board((vec2){6.0f, 0.0f}, chess_piece_type_knight, false);
    add_chess_piece_to_board((vec2){7.0f, 0.0f}, chess_piece_type_rook, false);
    for (u32 x = 0; x < 8; x++) {
        add_chess_piece_to_board((vec2){x, 1.0f}, chess_piece_type_pawn, false);
    }
    add_chess_piece_to_board((vec2){0.0f, 7.0f}, chess_piece_type_rook, true);
    add_chess_piece_to_board((vec2){1.0f, 7.0f}, chess_piece_type_knight, true);
    add_chess_piece_to_board((vec2){2.0f, 7.0f}, chess_piece_type_bishop, true);
    add_chess_piece_to_board((vec2){3.0f, 7.0f}, chess_piece_type_queen, true);
    add_chess_piece_to_board((vec2){4.0f, 7.0f}, chess_piece_type_king, true);
    add_chess_piece_to_board((vec2){5.0f, 7.0f}, chess_piece_type_bishop, true);
    add_chess_piece_to_board((vec2){6.0f, 7.0f}, chess_piece_type_knight, true);
    add_chess_piece_to_board((vec2){7.0f, 7.0f}, chess_piece_type_rook, true);
    for (u32 x = 0; x < 8; x++) {
        add_chess_piece_to_board((vec2){x, 6.0f}, chess_piece_type_pawn, true);
    }
}
void destroy_chess_board() {
    free(board_data.pieces);
}
void add_chess_piece_to_board(vec2 pos, chess_piece_type type, bool8 is_white) {
    if (pos.x >= BOARD_X_SIZE || pos.y >= BOARD_Y_SIZE) return;
    board_data.pieces[board_data.piece_count].type = type;
    board_data.pieces[board_data.piece_count].board_pos = pos;
    board_data.pieces[board_data.piece_count].is_white = is_white;
    if (board_data.pieces[board_data.piece_count].type == chess_piece_type_pawn) {
        board_data.pieces[board_data.piece_count].pawn_moved = false;
    }
    board_data.piece_count++;
}

void move_chess_piece_on_board(vec2 src_pos, vec2 dst_pos) {
    if (src_pos.x >= BOARD_X_SIZE || src_pos.y >= BOARD_Y_SIZE) return;
    if (dst_pos.x >= BOARD_X_SIZE || dst_pos.y >= BOARD_Y_SIZE) return;
    for (u32 i = 0; i < board_data.piece_count; i++) {
        if (board_data.pieces[i].board_pos.x == src_pos.x &&
            board_data.pieces[i].board_pos.y == src_pos.y) {
            board_data.pieces[i].board_pos = dst_pos;
        }
    }
}

chess_piece get_chess_piece_by_board_pos(vec2 board_pos) {
    chess_piece ret = {
        .board_pos = (vec2){-1.0f, -1.0f},
        .is_white = false,
        .type = chess_piece_type_none};
    if (board_pos.x >= BOARD_X_SIZE || board_pos.y >= BOARD_Y_SIZE) return ret;
    for (u32 i = 0; i < board_data.piece_count; i++) {
        if(board_data.pieces[i].type == chess_piece_type_none) continue;
        if (board_data.pieces[i].board_pos.x == board_pos.x && board_data.pieces[i].board_pos.y == board_pos.y) {
            return board_data.pieces[i];
        }
    }
    return ret;
}
void render_chess_pieces_on_board() {
    for (u32 i = 0; i < board_data.piece_count; i++) {
        if (board_data.pieces[i].type == chess_piece_type_none) continue;
        vec2 uv = (vec2){-1.0f, -1.0f};
        switch (board_data.pieces[i].type) {
            case chess_piece_type_pawn:
                uv = (vec2){0.0f, 0.0f};
                break;
            case chess_piece_type_bishop:
                uv = (vec2){4.0f, 0.0f};
                break;
            case chess_piece_type_knight:
                uv = (vec2){5.0f, 0.0f};
                break;
            case chess_piece_type_rook:
                uv = (vec2){1.0f, 0.0f};
                break;
            case chess_piece_type_queen:
                uv = (vec2){2.0f, 0.0f};
                break;
            case chess_piece_type_king:
                uv = (vec2){3.0f, 0.0f};
                break;
            default:
                break;
        }
        opengl_shader_bind(r_data.shader);
        opengl_shader_upload_int(r_data.shader, board_data.pieces[i].is_white, "u_chess_piece_white");
        render_quad(uv, (vec4){1.0f, 1.0f, 1.0f, 1.0f}, (vec2){((WINDOW_WIDTH / BOARD_Y_SIZE) / 2) + WINDOW_HEIGHT / BOARD_X_SIZE * board_data.pieces[i].board_pos.x, ((WINDOW_WIDTH / BOARD_Y_SIZE) / 2) + WINDOW_WIDTH / BOARD_Y_SIZE * board_data.pieces[i].board_pos.y}, (vec2){WINDOW_HEIGHT / BOARD_X_SIZE, WINDOW_WIDTH / BOARD_Y_SIZE});
    }
}

void render_quad_on_chess_board(vec4 color, vec2 board_pos, float scaling) {
    render_quad_color(color, (vec2){((WINDOW_WIDTH / BOARD_X_SIZE) / 2) + WINDOW_WIDTH / BOARD_X_SIZE * board_pos.x, ((WINDOW_HEIGHT / BOARD_Y_SIZE) / 2) + WINDOW_HEIGHT / BOARD_Y_SIZE * board_pos.y}, (vec2){(WINDOW_WIDTH / BOARD_X_SIZE) * scaling, (WINDOW_HEIGHT / BOARD_Y_SIZE) * scaling});
}

static bool8 is_chess_piece_move_possible(vec2 dst_pos) {
    return get_chess_piece_by_board_pos(dst_pos).type == chess_piece_type_none;
}

static u32 get_straight_full_board_moves(chess_piece piece, vec2* available_moves) {
    u32 moves_count = 0;
    for(u32 y = 1; y < BOARD_Y_SIZE; y++) {
        vec2 current_move = (vec2){piece.board_pos.x, piece.board_pos.y - y};
        if(current_move.x < 0 || current_move.x > BOARD_X_SIZE - 1  || current_move.y < 0 || current_move.y > BOARD_Y_SIZE - 1)
            continue;
        chess_piece current_piece = get_chess_piece_by_board_pos(current_move);
        if(!is_chess_piece_move_possible(current_move) && current_piece.is_white == piece.is_white) 
            break; 
        available_moves[moves_count++] = current_move;
        if(!is_chess_piece_move_possible(current_move) && current_piece.is_white != piece.is_white) 
            break; 
    }
    for(u32 y = 1; y < BOARD_Y_SIZE; y++) {
        vec2 current_move = (vec2){piece.board_pos.x, piece.board_pos.y + y};
        if(current_move.x < 0 || current_move.x > BOARD_X_SIZE - 1  || current_move.y < 0 || current_move.y > BOARD_Y_SIZE - 1)
            continue;
        chess_piece current_piece = get_chess_piece_by_board_pos(current_move);
        if(!is_chess_piece_move_possible(current_move) && current_piece.is_white == piece.is_white) 
            break; 
        available_moves[moves_count++] = current_move;
        if(!is_chess_piece_move_possible(current_move) && current_piece.is_white != piece.is_white) 
            break; 
    }
    for(u32 x = 1; x < BOARD_X_SIZE; x++) {
        vec2 current_move = (vec2){piece.board_pos.x - x, piece.board_pos.y};
        if(current_move.x < 0 || current_move.x > BOARD_X_SIZE - 1  || current_move.y < 0 || current_move.y > BOARD_Y_SIZE - 1)
            continue;
        chess_piece current_piece = get_chess_piece_by_board_pos(current_move);
        if(!is_chess_piece_move_possible(current_move) && current_piece.is_white == piece.is_white) 
            break; 
        available_moves[moves_count++] = current_move;
        if(!is_chess_piece_move_possible(current_move) && current_piece.is_white != piece.is_white) 
            break; 

    }
    for(u32 x = 1; x < BOARD_X_SIZE; x++) {
        vec2 current_move = (vec2){piece.board_pos.x + x, piece.board_pos.y};
        if(current_move.x < 0 || current_move.x > BOARD_X_SIZE - 1  || current_move.y < 0 || current_move.y > BOARD_Y_SIZE - 1)
            continue;
        chess_piece current_piece = get_chess_piece_by_board_pos(current_move);
        if(!is_chess_piece_move_possible(current_move) && current_piece.is_white == piece.is_white) 
            break; 
        available_moves[moves_count++] = current_move;
        if(!is_chess_piece_move_possible(current_move) && current_piece.is_white != piece.is_white) 
            break; 
    }
    return moves_count;
}
static u32 get_diagonal_full_board_moves(chess_piece piece, vec2* available_moves) {
    u32 moves_count = 0;
    for(u32 add = 1; add < BOARD_X_SIZE; add++) { 
            vec2 current_move = (vec2){piece.board_pos.x - add, piece.board_pos.y - add};
            if(current_move.x < 0 || current_move.x > BOARD_X_SIZE - 1  || current_move.y < 0 || current_move.y > BOARD_Y_SIZE - 1)
                continue;
            chess_piece current_piece = get_chess_piece_by_board_pos(current_move);
            if(!is_chess_piece_move_possible(current_move) && current_piece.is_white == piece.is_white) 
                break;
            available_moves[moves_count++] = current_move;
            if(!is_chess_piece_move_possible(current_move) && current_piece.is_white != piece.is_white)
                break;
        }
        for(u32 add = 1; add < BOARD_X_SIZE; add++) {
            vec2 current_move = (vec2){piece.board_pos.x + add, piece.board_pos.y - add};
            if(current_move.x < 0 || current_move.x > BOARD_X_SIZE - 1  || current_move.y < 0 || current_move.y > BOARD_Y_SIZE - 1)
                continue;
            chess_piece current_piece = get_chess_piece_by_board_pos(current_move);
            if(!is_chess_piece_move_possible(current_move) && current_piece.is_white == piece.is_white) 
                break;
            available_moves[moves_count++] = current_move;
            if(!is_chess_piece_move_possible(current_move) && current_piece.is_white != piece.is_white) 
                break;
        }
        for(u32 add = 1; add < BOARD_X_SIZE; add++) {
            vec2 current_move = (vec2){piece.board_pos.x - add, piece.board_pos.y + add};
            if(current_move.x < 0 || current_move.x > BOARD_X_SIZE - 1  || current_move.y < 0 || current_move.y > BOARD_Y_SIZE - 1)
                continue;
            chess_piece current_piece = get_chess_piece_by_board_pos(current_move);
            if(!is_chess_piece_move_possible(current_move) && current_piece.is_white == piece.is_white) 
                break;
            available_moves[moves_count++] = current_move;
            if(!is_chess_piece_move_possible(current_move) && current_piece.is_white != piece.is_white) 
                break;
        }
        for(u32 add = 1; add < BOARD_X_SIZE; add++) {
            vec2 current_move = (vec2){piece.board_pos.x + add, piece.board_pos.y + add};
            if(current_move.x < 0 || current_move.x > BOARD_X_SIZE - 1  || current_move.y < 0 || current_move.y > BOARD_Y_SIZE - 1)
                continue;
            chess_piece current_piece = get_chess_piece_by_board_pos(current_move);
            if(!is_chess_piece_move_possible(current_move) && current_piece.is_white == piece.is_white) 
                break;
            available_moves[moves_count++] = current_move;
            if(!is_chess_piece_move_possible(current_move) && current_piece.is_white != piece.is_white) 
                break;
        }
    return moves_count;
}

vec2 get_king_position(bool8 is_white) {
    for(u32 i = 0; i < board_data.piece_count; i++) {
        if(board_data.pieces[i].is_white == is_white && board_data.pieces[i].type == chess_piece_type_king) 
            return board_data.pieces[i].board_pos;
    }
    return (vec2){-1.0f, -1.0f};
}


bool8 is_king_in_check(bool8 white) {
    for(u32 i = 0; i < board_data.piece_count; i++) { 
        vec2 available_moves_for_piece[96];
        u32 available_moves_for_piece_count = get_available_moves_from_chess_piece(board_data.pieces[i], available_moves_for_piece);
        for(u32 j = 0; j < available_moves_for_piece_count; j++) {
            if((available_moves_for_piece[j].x == get_king_position(white).x && 
                available_moves_for_piece[j].y == get_king_position(white).y)) {
                return true;
            }
        }
    }
    return false;
}
bool8 is_king_in_check_after_move(vec2 src_move, vec2 dst_move, bool8 white) {
    for(u32 i = 0; i < board_data.piece_count; i++) {
        if(board_data.pieces[i].board_pos.x == src_move.x &&
            board_data.pieces[i].board_pos.y == src_move.y) {
            board_data.pieces[i].board_pos = dst_move;
            bool8 ret = is_king_in_check(!white);
            board_data.pieces[i].board_pos = src_move;
            return ret;
        }
    }
    return false;
}

u32 get_available_moves_from_chess_piece(chess_piece piece, vec2* available_moves) {
    u32 moves_count = 0;
    if (piece.type == chess_piece_type_pawn) {
        if (piece.is_white) {
            if (is_chess_piece_move_possible((vec2){piece.board_pos.x, piece.board_pos.y - 1.0f})) {
                available_moves[moves_count++] = (vec2){piece.board_pos.x, piece.board_pos.y - 1.0f};
                if (!piece.pawn_moved) {
                    if (is_chess_piece_move_possible((vec2){piece.board_pos.x, piece.board_pos.y - 2.0f}))
                        available_moves[moves_count++] = (vec2){piece.board_pos.x, piece.board_pos.y - 2.0f};
                }
            }
            chess_piece diagonal_left_neighbour = get_chess_piece_by_board_pos((vec2){piece.board_pos.x - 1.0f, piece.board_pos.y - 1.0f});
            if (diagonal_left_neighbour.type != chess_piece_type_none && !diagonal_left_neighbour.is_white) {
                available_moves[moves_count++] = diagonal_left_neighbour.board_pos;
            }

            chess_piece diagonal_right_neighbour = get_chess_piece_by_board_pos((vec2){piece.board_pos.x + 1.0f, piece.board_pos.y - 1.0f});
            if (diagonal_right_neighbour.type != chess_piece_type_none && !diagonal_right_neighbour.is_white) {
                available_moves[moves_count++] = diagonal_right_neighbour.board_pos;
            }
        } else {
            if (is_chess_piece_move_possible((vec2){piece.board_pos.x, piece.board_pos.y + 1.0f})) {
                available_moves[moves_count++] = (vec2){piece.board_pos.x, piece.board_pos.y + 1.0f};
                if (!piece.pawn_moved) {
                    if (is_chess_piece_move_possible((vec2){piece.board_pos.x, piece.board_pos.y + 2.0f}))
                        available_moves[moves_count++] = (vec2){piece.board_pos.x, piece.board_pos.y + 2.0f};
                }
            }

            chess_piece diagonal_left_neighbour = get_chess_piece_by_board_pos((vec2){piece.board_pos.x - 1.0f, piece.board_pos.y + 1.0f});
            if (diagonal_left_neighbour.type != chess_piece_type_none && diagonal_left_neighbour.is_white) {
                available_moves[moves_count++] = diagonal_left_neighbour.board_pos;
            }

            chess_piece diagonal_right_neighbour = get_chess_piece_by_board_pos((vec2){piece.board_pos.x + 1.0f, piece.board_pos.y + 1.0f});
            if (diagonal_right_neighbour.type != chess_piece_type_none && diagonal_right_neighbour.is_white) {
                available_moves[moves_count++] = diagonal_right_neighbour.board_pos;
            }
        }
    } else if (piece.type == chess_piece_type_rook) {
        moves_count = get_straight_full_board_moves(piece, available_moves);
    } else if(piece.type == chess_piece_type_bishop) {
        moves_count = get_diagonal_full_board_moves(piece, available_moves);
    } else if(piece.type == chess_piece_type_queen) {
        moves_count = get_straight_full_board_moves(piece, available_moves); 
        for(u32 add = 1; add < BOARD_X_SIZE; add++) { 
            vec2 current_move = (vec2){piece.board_pos.x - add, piece.board_pos.y - add};
            if(current_move.x < 0 || current_move.x > BOARD_X_SIZE - 1  || current_move.y < 0 || current_move.y > BOARD_Y_SIZE - 1)
                continue;
            chess_piece current_piece = get_chess_piece_by_board_pos(current_move);
            if(!is_chess_piece_move_possible(current_move) && current_piece.is_white == piece.is_white) 
                break;
            available_moves[moves_count++] = current_move;
            if(!is_chess_piece_move_possible(current_move) && current_piece.is_white != piece.is_white)
                break;
        }
        for(u32 add = 1; add < BOARD_X_SIZE; add++) {
            vec2 current_move = (vec2){piece.board_pos.x + add, piece.board_pos.y - add};
            if(current_move.x < 0 || current_move.x > BOARD_X_SIZE - 1  || current_move.y < 0 || current_move.y > BOARD_Y_SIZE - 1)
                continue;
            chess_piece current_piece = get_chess_piece_by_board_pos(current_move);
            if(!is_chess_piece_move_possible(current_move) && current_piece.is_white == piece.is_white) 
                break;
            available_moves[moves_count++] = current_move;
            if(!is_chess_piece_move_possible(current_move) && current_piece.is_white != piece.is_white) 
                break;
        }
        for(u32 add = 1; add < BOARD_X_SIZE; add++) {
            vec2 current_move = (vec2){piece.board_pos.x - add, piece.board_pos.y + add};
            if(current_move.x < 0 || current_move.x > BOARD_X_SIZE - 1  || current_move.y < 0 || current_move.y > BOARD_Y_SIZE - 1)
                continue;
            chess_piece current_piece = get_chess_piece_by_board_pos(current_move);
            if(!is_chess_piece_move_possible(current_move) && current_piece.is_white == piece.is_white) 
                break;
            available_moves[moves_count++] = current_move;
            if(!is_chess_piece_move_possible(current_move) && current_piece.is_white != piece.is_white) 
                break;
        }
        for(u32 add = 1; add < BOARD_X_SIZE; add++) {
            vec2 current_move = (vec2){piece.board_pos.x + add, piece.board_pos.y + add};
            if(current_move.x < 0 || current_move.x > BOARD_X_SIZE - 1  || current_move.y < 0 || current_move.y > BOARD_Y_SIZE - 1)
                continue;
            chess_piece current_piece = get_chess_piece_by_board_pos(current_move);
            if(!is_chess_piece_move_possible(current_move) && current_piece.is_white == piece.is_white) 
                break;
            available_moves[moves_count++] = current_move;
            if(!is_chess_piece_move_possible(current_move) && current_piece.is_white != piece.is_white) 
                break;
        }
    } else if(piece.type == chess_piece_type_king) {
        for(i32 x = -1; x <= 1; x++) {
            for(i32 y = -1; y <= 1; y++) {
                vec2 current_move = (vec2){piece.board_pos.x + x, piece.board_pos.y + y};
                if(current_move.x < 0 || current_move.x > BOARD_X_SIZE - 1  || current_move.y < 0 || current_move.y > BOARD_Y_SIZE - 1)
                    continue;
                chess_piece current_piece = get_chess_piece_by_board_pos(current_move);
                if(!is_chess_piece_move_possible(current_move) && current_piece.is_white == piece.is_white) 
                    continue; 
                available_moves[moves_count++] = current_move;
                if(!is_chess_piece_move_possible(current_move) && current_piece.is_white != piece.is_white) 
                    continue;

            }
        }
    } else if(piece.type == chess_piece_type_knight) {
        if(get_chess_piece_by_board_pos((vec2){piece.board_pos.x - 2.0f, piece.board_pos.y - 1.0f}).is_white != piece.is_white 
            || is_chess_piece_move_possible((vec2){piece.board_pos.x - 2.0f, piece.board_pos.y - 1.0f})) {
            available_moves[moves_count++] = (vec2){piece.board_pos.x - 2.0f, piece.board_pos.y - 1.0f};
        }
        if(get_chess_piece_by_board_pos((vec2){piece.board_pos.x - 1.0f, piece.board_pos.y - 2.0f}).is_white != piece.is_white 
            || is_chess_piece_move_possible((vec2){piece.board_pos.x - 1.0f, piece.board_pos.y - 2.0f})) {
            available_moves[moves_count++] = (vec2){piece.board_pos.x - 1.0f, piece.board_pos.y - 2.0f};
        }
        if(get_chess_piece_by_board_pos((vec2){piece.board_pos.x + 2.0f, piece.board_pos.y - 1.0f}).is_white != piece.is_white 
            || is_chess_piece_move_possible((vec2){piece.board_pos.x + 2.0f, piece.board_pos.y - 1.0f})) {
            available_moves[moves_count++] = (vec2){piece.board_pos.x + 2.0f, piece.board_pos.y - 1.0f};
        }
        if(get_chess_piece_by_board_pos((vec2){piece.board_pos.x + 1.0f, piece.board_pos.y - 2.0f}).is_white != piece.is_white
            || is_chess_piece_move_possible((vec2){piece.board_pos.x + 1.0f, piece.board_pos.y - 2.0f})) {
            available_moves[moves_count++] = (vec2){piece.board_pos.x + 1.0f, piece.board_pos.y - 2.0f};
        }
        if(get_chess_piece_by_board_pos((vec2){piece.board_pos.x - 2.0f, piece.board_pos.y + 1.0f}).is_white != piece.is_white 
            || is_chess_piece_move_possible((vec2){piece.board_pos.x - 2.0f, piece.board_pos.y + 1.0f})) {
            available_moves[moves_count++] = (vec2){piece.board_pos.x - 2.0f, piece.board_pos.y + 1.0f};
        }
        if(get_chess_piece_by_board_pos((vec2){piece.board_pos.x - 1.0f, piece.board_pos.y + 2.0f}).is_white != piece.is_white
            || is_chess_piece_move_possible((vec2){piece.board_pos.x - 1.0f, piece.board_pos.y + 2.0f})) {
            available_moves[moves_count++] = (vec2){piece.board_pos.x - 1.0f, piece.board_pos.y + 2.0f};
        }
        if(get_chess_piece_by_board_pos((vec2){piece.board_pos.x + 2.0f, piece.board_pos.y + 1.0f}).is_white != piece.is_white
            || is_chess_piece_move_possible((vec2){piece.board_pos.x + 2.0f, piece.board_pos.y + 1.0f})) {
            available_moves[moves_count++] = (vec2){piece.board_pos.x + 2.0f, piece.board_pos.y + 1.0f};
        }
        if(get_chess_piece_by_board_pos((vec2){piece.board_pos.x + 1.0f, piece.board_pos.y + 2.0f}).is_white != piece.is_white
            || is_chess_piece_move_possible((vec2){piece.board_pos.x + 1.0f, piece.board_pos.y + 2.0f})) {
            available_moves[moves_count++] = (vec2){piece.board_pos.x + 1.0f, piece.board_pos.y + 2.0f};
        }


    }
    return moves_count;
}


void remove_chess_piece_from_board(vec2 board_pos) {
    u32 index = 0;
    for (u32 i = 0; i < board_data.piece_count; i++) {
        if (board_data.pieces[i].board_pos.x == board_pos.x && board_data.pieces[i].board_pos.y == board_pos.y) {
            index = i;
        }
    }

    for (u32 i = index; i < board_data.piece_count - 1; i++) {
        board_data.pieces[i] = board_data.pieces[i + 1];
    }

    board_data.piece_count--;
}
