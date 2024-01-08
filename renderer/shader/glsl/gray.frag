
uniform sampler2D u_texture;
uniform float u_gray;

varying vec2 v_texCoord;

const vec3 lumaF = vec3(.299, .587, .114);

void main() {
	vec4 frag = texture2D(u_texture, v_texCoord);

	float luma = dot(frag.rgb, lumaF);
	frag.rgb = mix(frag.rgb, vec3(luma), u_gray);

	gl_FragColor = frag;
}
