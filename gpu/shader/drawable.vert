
// From gl_shader_manager.cc:140
uniform mat4 viewpMat;

// Texture and Offset
uniform vec2 texSize;
uniform vec2 transOffset;

attribute vec2 position;
attribute vec2 texCoord;

varying vec2 v_texCoord;

void main() {
	gl_Position = viewpMat * vec4(position + transOffset, 0, 1);

	v_texCoord = texCoord * texSize;
}
