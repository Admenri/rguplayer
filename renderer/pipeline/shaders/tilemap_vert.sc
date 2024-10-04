$input a_position, a_texcoord0, a_color0
$output v_texcoord0

#include "common/bgfx_shader.sh"

uniform vec4 u_offsetTexSize;

uniform vec4 u_tileSize_AnimateIndex;

const vec2 kAutotileArea = vec2(3.0, 28.0);

void main() {
	vec2 tex = a_texcoord0;

	// Animated area
	float addition = float(tex.x <= kAutotileArea.x * u_tileSize_AnimateIndex.x && tex.y <= kAutotileArea.y * u_tileSize_AnimateIndex.x);
	tex.x += 3.0 * u_tileSize_AnimateIndex.x * u_tileSize_AnimateIndex.y * addition;
	v_texcoord0 = vec2(tex.x * u_offsetTexSize.z, tex.y * u_offsetTexSize.w);

	vec2 transOffset = u_offsetTexSize.xy;
	gl_Position = mul(u_modelViewProj, a_position + vec4(transOffset, 0.0, 0.0));
}
