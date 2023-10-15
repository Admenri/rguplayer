// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_GLES2_SHADER_SHADER_MANAGER_H_
#define GPU_GLES2_SHADER_SHADER_MANAGER_H_

#include "base/math/math.h"
#include "gpu/gl_forward.h"
#include "gpu/gles2/meta/gles_meta.h"

namespace gpu {

enum ShaderAttribLocation : GLuint {
  Position = 0,
  TexCoord,
  Color,
};

class GLES2Shader {
 public:
  GLES2Shader();
  virtual ~GLES2Shader();

  GLES2Shader(const GLES2Shader&) = delete;
  GLES2Shader& operator=(const GLES2Shader&) = delete;

  void Bind();
  void Unbind();

 protected:
  virtual void Setup(const std::string& vertex_shader,
                     const std::string& frag_shader);

  virtual bool BindAttribLocation() { return false; }

  GLuint vertex_shader_;
  GLuint frag_shader_;
  GLuint program_;

 private:
  void CompileShader(GLuint gl_shader, const std::string& shader_source);
};

class GLES2ShaderBase : public GLES2Shader {
 public:
  GLES2ShaderBase();

  void SetProjectionMatrix(const base::Vec2i& size);

 protected:
  void Setup(const std::string& vertex_shader,
             const std::string& frag_shader) override;

  void SetTexture(GLint location, GLuint tex, uint16_t unit);

 private:
  GLint u_projectionMat_;
};

class BaseShader : public GLES2ShaderBase {
 public:
  BaseShader();

  void SetTextureSize(const base::Vec2& tex_size);
  void SetTransOffset(const base::Vec2& offset);
  void SetTexture(GLID<Texture> tex);

 private:
  GLint u_texSize_;
  GLint u_transOffset_;
  GLint u_texture_;
};

class TransformShader : public GLES2ShaderBase {
 public:
  TransformShader();

  void SetTextureSize(const base::Vec2& tex_size);
  void SetTransformMatrix(const float* mat4);
  void SetTexture(GLID<Texture> tex);

 private:
  GLint u_texSize_;
  GLint u_transformMat_;
  GLint u_texture_;
};

class TexBltShader : public GLES2ShaderBase {
 public:
  TexBltShader();

  void SetTextureSize(const base::Vec2& tex_size);
  void SetTransOffset(const base::Vec2& offset);
  void SetSrcTexture(GLID<Texture> tex);
  void SetDstTexture(GLID<Texture> tex);
  void SetOffsetScale(const base::Vec4& offset_scale);
  void SetOpacity(float opacity);

 private:
  GLint u_texSize_;
  GLint u_transOffset_;
  GLint u_texture_;
  GLint u_dst_texture_;
  GLint u_offset_scale_;
  GLint u_opacity_;
};

class ColorShader : public GLES2ShaderBase {
 public:
  ColorShader();

  void SetTransOffset(const base::Vec2& offset);

 private:
  GLint u_transOffset_;
};

}  // namespace gpu

#endif  // GPU_GLES2_SHADER_SHADER_MANAGER_H_