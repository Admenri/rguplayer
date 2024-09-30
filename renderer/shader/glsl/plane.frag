
uniform sampler2D u_texture;

uniform float u_opacity;
uniform vec4 u_color;
uniform vec4 u_tone;

in vec2 v_texCoord;

out vec4 fragColor;

const vec3 lumaF = vec3(.299, .587, .114);

void main() {
	vec4 frag = texture(u_texture, v_texCoord);

	float luma = dot(frag.rgb, lumaF);
	frag.rgb = mix(frag.rgb, vec3(luma), u_tone.w);
	frag.rgb += u_tone.rgb;

	frag.a *= u_opacity;

	frag.rgb = mix(frag.rgb, u_color.rgb, u_color.a);

	fragColor = frag;
}
