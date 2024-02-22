
uniform mat4 u_projectionMat;

uniform vec2 u_texSize;
uniform vec2 u_transOffset;

attribute vec4 a_position;
attribute vec2 a_texCoord;
attribute vec4 a_color;

varying vec2 v_texCoord;
varying vec4 v_color;

void main() {
	gl_Position = u_projectionMat * vec4(a_position.xy + u_transOffset, a_position.z, a_position.w);

	v_texCoord = a_texCoord * u_texSize;
	v_color = a_color;
}
