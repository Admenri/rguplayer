
uniform sampler2D u_texture;
uniform sampler2D u_dst_texture;

uniform vec4 u_offset_scale;
uniform float u_opacity;

varying vec2 v_texCoord;

void main() {
	vec4 src_frag = texture2D(u_texture, v_texCoord);
	vec2 dst_texCoord = (v_texCoord - u_offset_scale.xy) * u_offset_scale.zw;
	vec4 dst_frag = texture2D(u_dst_texture, dst_texCoord);

	vec4 result_frag;

	float src_alpha = src_frag * u_opacity;
	float dst_alpha = dst_frag * (1.0 - u_opacity);
	result_frag.a = src_alpha + dst_alpha;

	if (result_frag.a == 0.0)
		discard;

	result_frag.rgb = (src_alpha * src_frag + dst_alpha * dst_frag) / result_frag.a;

	gl_FragColor = result;
}
