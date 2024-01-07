
uniform sampler2D u_texture;

varying vec2 v_texCoord;
varying vec4 v_color;

void main() {
	vec4 frag = texture2D(u_texture, v_texCoord);

	gl_FragColor = frag;
	gl_FragColor.a *= v_color.a;
}
