#version 300 es
precision mediump float;

uniform sampler2D u_texture;

in vec2 v_texCoord;

out vec4 fragColor;

void main() {
	vec4 frag = texture(u_texture, v_texCoord);

	fragColor = frag;
}
