
uniform sampler2D u_texture;

uniform float u_opacity = 1.0;

varying vec2 v_texCoord;

void main() {
	vec4 frag = texture2D(u_texture, v_texCoord);

	gl_FragColor = frag;
	gl_FragColor.a *= u_opacity;
}
