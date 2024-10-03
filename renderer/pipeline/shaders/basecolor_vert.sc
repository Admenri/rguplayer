$input a_position, a_texcoord0, a_color0
$output v_color0

#include "common/bgfx_shader.sh"

void main() {
	gl_Position = mul(u_modelViewProj, a_position);
	v_color0 = a_color0;
}
