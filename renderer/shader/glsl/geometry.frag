#version 300 es
precision mediump float;

uniform sampler2D u_texture;

uniform float u_textureEmptyFlag;

in vec2 v_texCoord;
in vec4 v_color;

out vec4 fragColor;

void main() {
	vec4 frag = texture(u_texture, v_texCoord);
	frag.rgb = mix(frag.rgb, v_color.rgb, v_color.a);
	frag.a += u_textureEmptyFlag;

	fragColor = frag;
}
