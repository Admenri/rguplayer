#version 300 es
precision mediump float;

uniform mat4 u_projectionMat;

in vec4 a_position;

void main() {
	gl_Position = u_projectionMat * vec4(a_position.xy, 0.0, 1.0);
}
