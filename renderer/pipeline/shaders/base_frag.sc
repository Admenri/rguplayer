$input v_texcoord0

#include "common/bgfx_shader.sh"

SAMPLER2D(u_texture, 0);

void main() {
	gl_FragColor = texture2D(u_texture, v_texcoord0);
}
