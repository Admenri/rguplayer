
uniform float u_alpha;

varying vec4 v_color;

void main() {
	gl_FragColor = vec4(v_color.rgb * u_alpha, 1.0);
}
