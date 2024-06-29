#version 300 es
precision mediump float;

uniform mat4 u_projectionMat;

uniform vec2 u_texSize;
uniform mat4 u_transformMat;

in vec2 a_position;
in vec2 a_texCoord;

out vec2 v_texCoord;

void main() {
	gl_Position = u_projectionMat * u_transformMat * vec4(a_position, 0.0, 1.0);

	v_texCoord = a_texCoord * u_texSize;
}
