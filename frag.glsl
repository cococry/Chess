#version 460 core

out vec4 o_color;

in vec2 v_texcoord;

uniform sampler2D u_texture;

uniform int u_using_textures;
uniform int u_chess_piece_white;
uniform vec4 u_color;

void main() {
    if(u_using_textures == 0) {
        o_color = u_color;
    } else {
        o_color = texture(u_texture, v_texcoord) * u_color;
        if(u_chess_piece_white == 1) {
            if(o_color == vec4(0.0, 0.0, 0.0, 1.0)) {
                o_color = vec4(1.0, 1.0, 1.0, 1.0);
            }
        }
    }
}
