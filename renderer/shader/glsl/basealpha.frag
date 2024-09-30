
uniform sampler2D u_texture;

in vec2 v_texCoord;
in vec4 v_color;

out vec4 fragColor;

void main() {
	vec4 frag = texture(u_texture, v_texCoord);

	fragColor = frag;
	fragColor.a *= v_color.a;
}
