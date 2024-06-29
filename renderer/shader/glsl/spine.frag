#version 300 es
precision mediump float;

uniform sampler2D u_texture;

in vec2 v_texCoord;
in vec4 v_color;

out vec4 fragColor;

void main() {
	fragColor = v_color * texture(u_texture, v_texCoord);
}
