
uniform sampler2D u_textureY;
uniform sampler2D u_textureU;
uniform sampler2D u_textureV;

in vec2 v_texCoord;

out vec4 fragColor;

/* YUV offset */
const vec3 kYUVOffset = vec3(0, -0.501960814, -0.501960814);

/* RGB coefficients */
const mat3 kYUVMatrix = mat3(1,       1,        1,
                             0,      -0.3441,   1.772,
                             1.402,  -0.7141,   0);

void main() {
  vec3 yuv;
	yuv.x = texture(u_textureY, v_texCoord).r;
	yuv.y = texture(u_textureU, v_texCoord).r;
	yuv.z = texture(u_textureV, v_texCoord).r;
	yuv += kYUVOffset;
	vec3 rgb = kYUVMatrix * yuv;

	fragColor = vec4(rgb, 1.0);
}
