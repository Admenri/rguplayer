
uniform sampler2D bitmap_texture;

varying vec2 v_texCoord;

void main() {
	gl_FragColor = texture2D(bitmap_texture, v_texCoord);
}
