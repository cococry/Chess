#version 460 core

out vec4 o_color;

in vec2 v_texcoord;

uniform sampler2D u_texture;

uniform int u_using_textures;
uniform vec4 u_color;

void main() {
    if(u_using_textures == 0) {
        o_color = u_color;
    } else {
        o_color = texture(u_texture, v_texcoord) * u_color;
    }
}
