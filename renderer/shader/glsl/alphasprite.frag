
uniform sampler2D u_texture;
uniform float u_opacity;

in vec2 v_texCoord;

out vec4 fragColor;

void main() {
	vec4 frag = texture(u_texture, v_texCoord);

	/* Opacity */
	frag.a *= u_opacity;
	fragColor = frag;
}
