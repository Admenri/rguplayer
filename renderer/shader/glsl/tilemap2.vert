
uniform mat4 u_projectionMat;

uniform vec2 u_texSize;
uniform vec2 u_transOffset;

uniform vec2 u_autotileAnimationOffset;
uniform float u_tileSize;

in vec4 a_position;
in vec2 a_texCoord;

out vec2 v_texCoord;

const vec2 k_regularArea = vec2(12.0, 12.0);
const vec4 k_waterfallArea = vec4(12.0, 16.0, 4.0, 12.0);
const vec4 k_waterfallAutotileArea = vec4(12.0, 16.0, 2.0, 6.0);

float posInArea(vec2 pos, vec4 area) {
	return float(pos.x >= area.x && pos.y >= area.y && pos.x <= (area.x + area.z) && pos.y <= (area.y + area.w));
}

void main() {
	vec2 tex = a_texCoord;
	float addition = 0.0;

	// Regular area
	addition = float(tex.x <= k_regularArea.x * u_tileSize && tex.y <= k_regularArea.y * u_tileSize);
	tex.x += u_autotileAnimationOffset.x * addition;

	// Waterfall area
	addition = posInArea(tex, k_waterfallArea) - posInArea(tex, k_waterfallAutotileArea);
	tex.y += u_autotileAnimationOffset.y * addition;

	gl_Position = u_projectionMat * vec4(a_position.xy + u_transOffset, 0.0, 1.0);

	v_texCoord = tex * u_texSize;
}
