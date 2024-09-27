$input v_texcoord0

#include "common/bgfx_shader.sh"

SAMPLER2D(u_texture, 0);
SAMPLER2D(u_dstTexture, 1);

uniform vec4 u_offsetScale;
uniform vec4 u_opacity;

void main() {
	vec2 offset = v_texcoord0 - u_offsetScale.xy;
	vec2 dst_texCoord = vec2(offset.x * u_offsetScale.z, offset.y * u_offsetScale.w);

	vec4 src_frag = texture2D(u_texture, v_texcoord0);
	vec4 dst_frag = texture2D(u_dstTexture, dst_texCoord);

	vec4 result_frag;

	float src_alpha = src_frag.a * u_opacity.x;
	float dst_alpha = dst_frag.a * (1.0 - src_alpha);
	result_frag.a = src_alpha + dst_alpha;

	if (result_frag.a == 0.0)
		result_frag.rgb = src_frag.rgb;
	else
		result_frag.rgb = (src_alpha * src_frag.rgb + dst_alpha * dst_frag.rgb) / result_frag.a;

	gl_FragColor = result_frag;
}
