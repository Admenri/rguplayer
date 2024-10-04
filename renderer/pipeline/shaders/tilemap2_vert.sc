$input a_position, a_texcoord0, a_color0
$output v_texcoord0

#include "common/bgfx_shader.sh"

uniform vec4 u_offsetTexSize;

uniform vec4 u_tileSize;
uniform vec4 u_autotileAnimationOffset;

const vec2 kAutotileArea = vec2(3.0, 28.0);

const vec2 kRegularArea = vec2(12.0, 12.0);
const vec4 kWaterfallArea = vec4(12.0, 16.0, 4.0, 12.0);
const vec4 kWaterfallAutotileArea = vec4(12.0, 16.0, 2.0, 6.0);

float posInArea(vec2 pos, vec4 area) {
	return float(pos.x >= area.x && pos.y >= area.y && pos.x <= (area.x + area.z) && pos.y <= (area.y + area.w));
}

void main() {
	vec2 tex = a_texcoord0;
	float addition = 0.0;

	// Regular area
	addition = float(tex.x <= kRegularArea.x * u_tileSize.x && tex.y <= kRegularArea.y * u_tileSize.x);
	tex.x += u_autotileAnimationOffset.x * addition;

	// Waterfall area
	addition = posInArea(tex, kWaterfallArea) - posInArea(tex, kWaterfallAutotileArea);
	tex.y += u_autotileAnimationOffset.y * addition;
	v_texcoord0 = vec2(tex.x * u_offsetTexSize.z, tex.y * u_offsetTexSize.w);

	vec2 transOffset = u_offsetTexSize.xy;
	gl_Position = mul(u_modelViewProj, a_position + vec4(transOffset, 0.0, 0.0));
}
