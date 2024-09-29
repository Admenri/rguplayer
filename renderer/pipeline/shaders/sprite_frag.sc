$input v_texcoord0

#include "common/bgfx_shader.sh"

SAMPLER2D(u_texture, 0);

uniform vec4 u_offsetTexSize;

uniform vec4 u_color;
uniform vec4 u_tone;

uniform vec4 u_drawInfo;

const vec3 lumaF = vec3(.299, .587, .114);

void main() {
	vec4 frag = texture2D(u_texture, v_texcoord0);

	/* Tone */
	float luma = dot(frag.rgb, lumaF);
	frag.rgb = mix(frag.rgb, vec3(luma, luma, luma), u_tone.w);
	frag.rgb += u_tone.rgb;

	/* Alpha */
	frag.a *= u_drawInfo.x;

	/* Color */
	frag.rgb = mix(frag.rgb, u_color.rgb, u_color.a);

	/* Bush depth alpha */
	float currentPos = v_texcoord0.y / u_offsetTexSize.w;
	float underBush = float(u_drawInfo.y > currentPos);
	frag.a *= clamp(u_drawInfo.z + underBush, 0.0, 1.0);

	gl_FragColor = frag;
}
