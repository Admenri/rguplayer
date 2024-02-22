
uniform sampler2D u_texture;
uniform float u_opacity;

varying vec2 v_texCoord;

void main() {
	vec4 frag = texture2D(u_texture, v_texCoord);

	/* Opacity */
	frag.a *= u_opacity;

	gl_FragColor = frag;
}
