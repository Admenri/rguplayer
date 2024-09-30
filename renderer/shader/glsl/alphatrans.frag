
uniform sampler2D u_frozenTexture;
uniform sampler2D u_currentTexture;
uniform float u_progress;

in vec2 v_texCoord;

out vec4 fragColor;

void main() {
	vec4 frozenFrag = texture(u_frozenTexture, v_texCoord);
	vec4 currentFrag = texture(u_currentTexture, v_texCoord);

	fragColor = mix(frozenFrag, currentFrag, u_progress);
}
