$input a_position, a_texcoord0
$output v_texcoord0

#include "common/bgfx_shader.sh"

uniform mat4 u_transformMat;
uniform vec4 u_offsetTexSize;

void main() {
	vec2 transOffset = u_offsetTexSize.xy;
	vec4 transPos = mul(u_transformMat, a_position + vec4(transOffset, 0.0, 0.0));
	gl_Position = mul(u_modelViewProj, transPos);
	v_texcoord0 = vec2(a_texcoord0.x * u_offsetTexSize.z, a_texcoord0.y * u_offsetTexSize.w);
}
