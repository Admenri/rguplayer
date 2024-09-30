
uniform sampler2D u_texture;
uniform sampler2D u_dst_texture;

uniform vec4 u_offset_scale;
uniform float u_opacity;

in vec2 v_texCoord;

out vec4 fragColor;

void main() {
	vec2 dst_texCoord = (v_texCoord - u_offset_scale.xy) * u_offset_scale.zw;

	vec4 src_frag = texture(u_texture, v_texCoord);
	vec4 dst_frag = texture(u_dst_texture, dst_texCoord);

	vec4 result_frag;

	float src_alpha = src_frag.a * u_opacity;
	float dst_alpha = dst_frag.a * (1.0 - src_alpha);
	result_frag.a = src_alpha + dst_alpha;

	if (result_frag.a == 0.0)
		result_frag.rgb = src_frag.rgb;
	else
		result_frag.rgb = (src_alpha * src_frag.rgb + dst_alpha * dst_frag.rgb) / result_frag.a;

	fragColor = result_frag;
}
