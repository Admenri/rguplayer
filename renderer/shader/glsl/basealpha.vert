#version 300 es
precision mediump float;

uniform mat4 u_projectionMat;

uniform vec2 u_texSize;
uniform vec2 u_transOffset;

in vec2 a_position;
in vec2 a_texCoord;
in vec4 a_color;

out vec2 v_texCoord;
out vec4 v_color;

void main() {
	gl_Position = u_projectionMat * vec4(a_position + u_transOffset, 0.0, 1.0);

	v_texCoord = a_texCoord * u_texSize;
	v_color = a_color;
}
