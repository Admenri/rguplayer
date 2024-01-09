
uniform sampler2D u_frozenTexture;
uniform sampler2D u_currentTexture;
uniform float u_progress;

varying vec2 v_texCoord;

void main() {
	vec4 frozenFrag = texture2D(u_frozenTexture, v_texCoord);
	vec4 currentFrag = texture2D(u_currentTexture, v_texCoord);

	gl_FragColor = mix(frozenFrag, currentFrag, u_progress);
}
