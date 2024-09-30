
uniform sampler2D u_texture;

uniform vec2 u_texSize;

uniform vec4 u_color;
uniform vec4 u_tone;
uniform float u_opacity;

uniform float u_bushDepth;
uniform float u_bushOpacity;

const vec3 lumaF = vec3(.299, .587, .114);

in vec2 v_texCoord;

out vec4 fragColor;

void main() {
	vec4 frag = texture(u_texture, v_texCoord);

	/* Tone */
	float luma = dot(frag.rgb, lumaF);
	frag.rgb = mix(frag.rgb, vec3(luma), u_tone.w);
	frag.rgb += u_tone.rgb;

	/* Opacity */
	frag.a *= u_opacity;

	/* Color */
	frag.rgb = mix(frag.rgb, u_color.rgb, u_color.a);

	/* Bush depth alpha */
	float currentPos = v_texCoord.y / u_texSize.y;
	float underBush = float(u_bushDepth > currentPos);
	frag.a *= clamp(u_bushOpacity + underBush, 0.0, 1.0);

	fragColor = frag;
}
