$input v_texcoord0, v_color0

#include "common/bgfx_shader.sh"

SAMPLER2D(u_texture, 0);

void main() {
	vec4 frag = texture2D(u_texture, v_texcoord0);
	frag.rgb = mix(frag.rgb, v_color0.rgb, v_color0.a);

	gl_FragColor = frag;
}
