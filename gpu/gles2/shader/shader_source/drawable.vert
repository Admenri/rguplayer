
// From gl_shader_manager.cc:140
uniform mat4 viewpMat;

// Texture size calc and Transform
uniform mat4 transformMat;
uniform vec2 texSize;

attribute vec2 position;
attribute vec2 texCoord;

varying vec2 v_texCoord;

void main() {
	gl_Position = viewpMat * transformMat * vec4(position, 0, 1);

	v_texCoord = texCoord * texSize;
}
