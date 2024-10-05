$input v_texcoord0

#include "common/bgfx_shader.sh"

SAMPLER2D(u_frozenTexture, 0);
SAMPLER2D(u_currentTexture, 1);

uniform vec4 u_progress;

void main() {
	vec4 frozenFrag = texture2D(u_frozenTexture, v_texcoord0);
	vec4 currentFrag = texture2D(u_currentTexture, v_texcoord0);

	gl_FragColor = mix(frozenFrag, currentFrag, u_progress.x);
}
