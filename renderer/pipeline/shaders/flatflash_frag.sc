$input v_color0

#include "common/bgfx_shader.sh"

uniform vec4 u_alpha;

void main() {
	gl_FragColor = vec4(v_color0.rgb * u_alpha.x, 1.0);
}
