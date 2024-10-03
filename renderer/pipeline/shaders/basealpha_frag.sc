$input v_texcoord0, v_color0

#include "common/bgfx_shader.sh"

SAMPLER2D(u_texture, 0);

void main() {
	gl_FragColor = texture2D(u_texture, v_texcoord0);
	gl_FragColor.a *= v_color0.a;
}
