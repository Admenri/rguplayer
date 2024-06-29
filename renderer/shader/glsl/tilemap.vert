#version 300 es
precision mediump float;

uniform mat4 u_projectionMat;

uniform vec2 u_texSize;
uniform vec2 u_transOffset;

uniform float u_tileSize;
uniform float u_animateIndex;

in vec2 a_position;
in vec2 a_texCoord;

out vec2 v_texCoord;

const vec2 k_autotileArea = vec2(3.0, 28.0);

void main() {
	vec2 tex = a_texCoord;

	// Animated area
	float addition = float(tex.x <= k_autotileArea.x * u_tileSize && tex.y <= k_autotileArea.y * u_tileSize);
	tex.x += 3.0 * u_tileSize * u_animateIndex * addition;

	gl_Position = u_projectionMat * vec4(a_position + u_transOffset, 0.0, 1.0);

	v_texCoord = tex * u_texSize;
}
