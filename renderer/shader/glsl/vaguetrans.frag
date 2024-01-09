
uniform sampler2D u_frozenTexture;
uniform sampler2D u_currentTexture;
uniform sampler2D u_transTexture;
uniform float u_progress;
uniform float u_vague;

varying vec2 v_texCoord;

void main() {
	vec4 frozenFrag = texture2D(u_frozenTexture, v_texCoord);
	vec4 currentFrag = texture2D(u_currentTexture, v_texCoord);
	float transSample = texture2D(u_transTexture, v_texCoord).r;

	transSample = clamp(transSample, u_progress, u_progress + u_vague);
	float mixAlpha = (transSample - u_progress) / u_vague;

	gl_FragColor = mix(frozenFrag, currentFrag, mixAlpha);
}
