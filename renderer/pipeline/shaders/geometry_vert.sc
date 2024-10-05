$input a_position, a_texcoord0, a_color0
$output v_texcoord0, v_color0

#include "common/bgfx_shader.sh"

uniform vec4 u_offsetTexSize;

void main() {
	vec2 transOffset = u_offsetTexSize.xy;
	gl_Position = mul(u_modelViewProj, a_position + vec4(transOffset, 0.0, 0.0));
	v_texcoord0 = vec2(a_texcoord0.x * u_offsetTexSize.z, a_texcoord0.y * u_offsetTexSize.w);
	v_color0 = a_color0;
}
