$input v_texcoord0

#include "common/bgfx_shader.sh"

SAMPLER2D(u_frozenTexture, 0);
SAMPLER2D(u_currentTexture, 1);
SAMPLER2D(u_transTexture, 2);

uniform vec4 u_progressVague;

void main() {
	vec4 frozenFrag = texture2D(u_frozenTexture, v_texcoord0);
	vec4 currentFrag = texture2D(u_currentTexture, v_texcoord0);
	float transSample = texture2D(u_transTexture, v_texcoord0).r;

	transSample = clamp(transSample, u_progressVague.x, u_progressVague.x + u_progressVague.y);
	float mixAlpha = (transSample - u_progressVague.x) / u_progressVague.y;

	gl_FragColor = mix(currentFrag, frozenFrag, mixAlpha);
}
