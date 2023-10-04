
// From gl_shader_manager.cc:140
uniform mat4 viewpMat;

uniform vec2 transOffset;

attribute vec2 position;
attribute vec2 texCoord;
attribute vec4 color;

varying vec4 v_color;

void main() {
	gl_Position = viewpMat * vec4(position + transOffset, 0, 1);

	v_color = color;
}
