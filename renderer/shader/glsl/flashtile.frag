
uniform float u_alpha;

in vec4 v_color;

out vec4 fragColor;

void main() {
	fragColor = vec4(v_color.rgb * u_alpha, 1.0);
}
