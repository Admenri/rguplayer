#version 300 es
precision mediump float;

uniform sampler2D u_frozenTexture;
uniform sampler2D u_currentTexture;
uniform sampler2D u_transTexture;
uniform float u_progress;
uniform float u_vague;

in vec2 v_texCoord;

out vec4 fragColor;

void main() {
	vec4 frozenFrag = texture(u_frozenTexture, v_texCoord);
	vec4 currentFrag = texture(u_currentTexture, v_texCoord);
	float transSample = texture(u_transTexture, v_texCoord).r;

	transSample = clamp(transSample, u_progress, u_progress + u_vague);
	float mixAlpha = (transSample - u_progress) / u_vague;

	fragColor = mix(currentFrag, frozenFrag, mixAlpha);
}
