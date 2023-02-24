#pragma once

/* ======================== */
/*   STANDARD INT DEFINES   */
/* ======================== */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;

typedef unsigned char bool8;
typedef unsigned int bool32;

#define false 0
#define true 1
#define exit_failure 1
#define exit_success 0

/* ======================== */
/*    LINEAR ALGEBRA API    */
/* ======================== */

typedef struct {
    float x, y;
} vec2;

typedef struct {
    float x, y, z;
} vec3;

typedef struct {
    float x, y, z, w;
} vec4;

typedef struct {
    vec4 row1, row2, row3, row4;
} mat4;

vec2 vec2_add(vec2 a, vec2 b);
vec2 vec2_sub(vec2 a, vec2 b);
vec2 vec2_mul(vec2 a, vec2 b);
vec2 vec2_div(vec2 a, vec2 b);

vec3 vec3_add(vec3 a, vec3 b);
vec3 vec3_sub(vec3 a, vec3 b);
vec3 vec3_mul(vec3 a, vec3 b);
vec3 vec3_div(vec3 a, vec3 b); 

vec4 vec4_add(vec4 a, vec4 b);
vec4 vec4_sub(vec4 a, vec4 b);
vec4 vec4_mul(vec4 a, vec4 b);
vec4 vec4_div(vec4 a, vec4 b);

float* vec2_value_ptr(vec2 v);
float* vec3_value_ptr(vec3 v);
float* vec4_value_ptr(vec4 v);
float* mat4_value_ptr(mat4 m);

vec4 vec4_scaler_mul(vec4 v, float k);

vec4 vec4_mul_mat4(vec4 v, mat4 m);

mat4 mat4_identity();

mat4 mat4_mul(mat4 m1, mat4 m2);

mat4 mat4_translate(mat4 m, vec3 v);

mat4 mat4_scale(mat4 m, vec3 v);

mat4 mat4_orthographic(float left, float right, float bottom, float top);
