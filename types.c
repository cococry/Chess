#include "types.h"
#include <stdlib.h>

vec2 vec2_add(vec2 a, vec2 b) {
    return (vec2){ a.x + b.x, a.y + b.y };
}

vec2 vec2_sub(vec2 a, vec2 b) {
    return (vec2){ a.x - b.x, a.y - b.y };
}

vec2 vec2_mul(vec2 a, vec2 b) {
    return (vec2){ a.x * b.x, a.y * b.y };
}

vec2 vec2_div(vec2 a, vec2 b) {
    return (vec2){ a.x / b.x, a.y / b.y };
}

vec3 vec3_add(vec3 a, vec3 b) {
    return (vec3){ a.x + b.x, a.y + b.y, a.z + b.z };
}
vec3 vec3_sub(vec3 a, vec3 b) {
    return (vec3){ a.x - b.x, a.y - b.y, a.z - b.z };

}
vec3 vec3_mul(vec3 a, vec3 b) {
    return (vec3){ a.x * b.x, a.y * b.y, a.z * b.z };
}
vec3 vec3_div(vec3 a, vec3 b) {
    return (vec3){ a.x / b.x, a.y / b.y, a.z / b.z };
}
vec4 vec4_add(vec4 a, vec4 b) {
    return (vec4){ a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
}
vec4 vec4_sub(vec4 a, vec4 b) {
    return (vec4){ a.x - b.x, a.y - b.y, a.z - b.z, a.w + b.w };
}
vec4 vec4_mul(vec4 a, vec4 b) {
    return (vec4){ a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w };
}
vec4 vec4_div(vec4 a, vec4 b) {
    return (vec4){ a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w };
}


vec4 vec4_scaler_mul(vec4 v, float k) {
    return (vec4){ v.x * k, v.y * k, v.z * k, v.w * k };
}

mat4 mat4_identity() {
    mat4 ret = (mat4){
        (vec4){1.0f, 0.0f, 0.0f, 0.0f},
        (vec4){0.0f, 1.0f, 0.0f, 0.0f},
        (vec4){0.0f, 0.0f, 1.0f, 0.0f},
        (vec4){0.0f, 0.0f, 0.0f, 1.0f}};
    return ret;
 }


mat4 mat4_mul(mat4 m1, mat4 m2) {
    vec4 SrcA0 = m1.row1;
    vec4 SrcA1 = m1.row2;
    vec4 SrcA2 = m1.row3;
    vec4 SrcA3 = m1.row4;

    vec4 SrcB0 = m2.row1;
    vec4 SrcB1 = m2.row2;
    vec4 SrcB2 = m2.row3;
    vec4 SrcB3 = m2.row4;

    mat4 ret;

    ret.row1 = vec4_add(vec4_add(
                            vec4_scaler_mul(SrcA0, SrcB0.x),
                            vec4_scaler_mul(SrcA1, SrcB0.y)),
                        vec4_add(
                            vec4_scaler_mul(SrcA2, SrcB0.z),
                            vec4_scaler_mul(SrcA3, SrcB0.w)));
    ret.row2 = vec4_add(vec4_add(
                            vec4_scaler_mul(SrcA0, SrcB1.x),
                            vec4_scaler_mul(SrcA1, SrcB1.y)),
                        vec4_add(
                            vec4_scaler_mul(SrcA2, SrcB1.z),
                            vec4_scaler_mul(SrcA3, SrcB1.w)));
    ret.row3 = vec4_add(vec4_add(
                            vec4_scaler_mul(SrcA0, SrcB2.x),
                            vec4_scaler_mul(SrcA1, SrcB2.y)),
                        vec4_add(
                            vec4_scaler_mul(SrcA2, SrcB2.z),
                            vec4_scaler_mul(SrcA3, SrcB2.w)));
    ret.row4 = vec4_add(vec4_add(
                            vec4_scaler_mul(SrcA0, SrcB3.x),
                            vec4_scaler_mul(SrcA1, SrcB3.y)),
                        vec4_add(
                            vec4_scaler_mul(SrcA2, SrcB3.z),
                            vec4_scaler_mul(SrcA3, SrcB3.w)));
    return ret;
}


float* vec2_value_ptr(vec2 v) {
    float* ret = malloc(sizeof(float) * 2);

    ret[0] = v.x;
    ret[1] = v.y;

    return ret;
}
float* vec3_value_ptr(vec3 v) {
    float* ret = malloc(sizeof(float) * 3);

    ret[0] = v.x;
    ret[1] = v.y;
    ret[2] = v.z;

    return ret;
}
float* vec4_value_ptr(vec4 v) {

    float* ret = malloc(sizeof(float) * 4);

    ret[0] = v.x;
    ret[1] = v.y;
    ret[2] = v.z;
    ret[3] = v.w;

    return ret;
}

float* mat4_value_ptr(mat4 m) {
    float* ret = malloc(sizeof(mat4));

    ret[0] = m.row1.x;
    ret[1] = m.row1.y;
    ret[2] = m.row1.z;
    ret[3] = m.row1.w;

    ret[4] = m.row2.x;
    ret[5] = m.row2.y;
    ret[6] = m.row2.z;
    ret[7] = m.row2.w;

    ret[8] = m.row3.x;
    ret[9] = m.row3.y;
    ret[10] = m.row3.z;
    ret[11] = m.row3.w;

    ret[12] = m.row4.x;
    ret[13] = m.row4.y;
    ret[14] = m.row4.z;
    ret[15] = m.row4.w;

    return ret;
}



vec4 vec4_mul_mat4(vec4 v, mat4 m) {
    vec4 Mov0 = (vec4){v.x, v.x, v.x, v.x};
    vec4 Mov1 = (vec4){v.y, v.y, v.y, v.y};
    vec4 Mul0 = vec4_mul(m.row1, Mov0);
    vec4 Mul1 = vec4_mul(m.row2, Mov1);
    vec4 Add0 = vec4_add(Mul0, Mul1);
    vec4 Mov2 = (vec4){v.z, v.z, v.z, v.z};
    vec4 Mov3 = (vec4){v.w, v.w, v.w, v.w};
    vec4 Mul2 = vec4_mul(m.row3, Mov2);
    vec4 Mul3 = vec4_mul(m.row4, Mov3);
    vec4 Add1 = vec4_add(Mul2, Mul3);
    vec4 Add2 = vec4_add(Add0, Add1);

    return Add2;
}


mat4 mat4_translate(mat4 m, vec3 v) {
    mat4 ret = m;
    ret.row4 = vec4_add(vec4_add(
                            vec4_scaler_mul(m.row1, v.x),
                            vec4_scaler_mul(m.row2, v.y)),
                        vec4_add(
                            vec4_scaler_mul(m.row3, v.z),
                            m.row4));
    return ret;
}

mat4 mat4_scale(mat4 m, vec3 v) {
    mat4 ret;
    ret.row1 = vec4_scaler_mul(m.row1, v.x);
    ret.row2 = vec4_scaler_mul(m.row2, v.x);
    ret.row3 = vec4_scaler_mul(m.row3, v.x);
    ret.row4 = m.row4;
    return ret;
}

mat4 mat4_orthographic(float left, float right, float bottom, float top) {
    mat4 ret = mat4_identity();
    ret.row1.x = 2 / (right - left);
    ret.row2.y = 2 / (top - bottom);
    ret.row3.z = -1;
    ret.row4.x = -(right + left) / (right - left);
    ret.row4.y = -(top + bottom) / (top - bottom);

    return ret;
}
