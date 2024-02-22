
uniform sampler2D u_texture;

uniform float u_textureEmptyFlag;

varying vec2 v_texCoord;
varying vec4 v_color;

void main() {
	vec4 frag = texture2D(u_texture, v_texCoord);
	frag.rgb = mix(frag.rgb, v_color.rgb, v_color.a);
	frag.a += u_textureEmptyFlag;

	gl_FragColor = frag;
}
