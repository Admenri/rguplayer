
uniform mat4 u_projectionMat;

uniform vec2 u_texSize;
uniform vec2 u_transOffset;

in vec4 a_position;
in vec2 a_texCoord;

out vec2 v_texCoord;

void main() {
	gl_Position = u_projectionMat * vec4(a_position.xy + u_transOffset, 0.0, 1.0);

	v_texCoord = a_texCoord * u_texSize;
}
