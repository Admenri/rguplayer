$input a_position, a_color0
$output v_color0

#include "common/bgfx_shader.sh"

uniform vec4 u_offsetTexSize;

void main() {
	vec2 transOffset = u_offsetTexSize.xy;
	gl_Position = mul(u_modelViewProj, a_position + vec4(transOffset, 0.0, 0.0));
	v_color0 = a_color0;
}
