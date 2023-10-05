
uniform sampler2D src_texture, dst_texture;

uniform vec4 subRect;
uniform float opacity;

varying vec2 v_texCoord;

void main() {
	vec2 srcCoord = v_texCoord;
	vec2 dstCoord = (srcCoord - subRect.xy) * subRect.zw;

	vec4 srcColor = texture2D(src_texture, srcCoord);
	vec4 dstColor = texture2D(dst_texture, dstCoord);

	float srcAlpha = srcColor.a * opacity;
	float dstAlpha = dstColor.a * (1.0 - srcAlpha);
	float resultAlpha = srcAlpha + dstAlpha;

	if (resultAlpha == 0.0)
		gl_FragColor = vec4(srcColor.rgb, 0.0);
	else
		gl_FragColor = vec4((srcAlpha * srcColor.rgb + dstAlpha * dstColor.rgb) / resultAlpha, resultAlpha);
}
