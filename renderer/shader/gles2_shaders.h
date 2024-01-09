// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef RENDERER_SHADER_GLES2_SHADERS_H_
#define RENDERER_SHADER_GLES2_SHADERS_H_

#include "base/math/math.h"
#include "base/math/transform.h"
#include "renderer/context/gles2_context.h"
#include "renderer/meta/gles2meta.h"

namespace renderer {

class GLES2Shader {
 public:
  enum AttribLocation : GLuint {
    Position = 0,
    TexCoord,
    Color,
  };

  GLES2Shader();
  ~GLES2Shader();

  GLES2Shader(const GLES2Shader&) = delete;
  GLES2Shader& operator=(const GLES2Shader&) = delete;

  void Bind();
  void Unbind();

  GLuint program() { return program_; }

 protected:
  virtual bool Setup(const std::string& vertex_shader,
                     const std::string& vertex_name,
                     const std::string& frag_shader,
                     const std::string& frag_name);
  virtual bool BindAttribLocation() { return true; }

 private:
  bool CompileShader(GLuint glshader,
                     const std::string& shader_source,
                     const std::string& shader_name);

  GLuint vertex_shader_;
  GLuint frag_shader_;
  GLuint program_;
};

class GLES2ShaderBase : public GLES2Shader {
 public:
  GLES2ShaderBase();

  void SetProjectionMatrix(const base::Vec2i& size);

 protected:
  bool Setup(const std::string& vertex_shader,
             const std::string& vertex_name,
             const std::string& frag_shader,
             const std::string& frag_name) override;
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

class BaseAlphaShader : public GLES2ShaderBase {
 public:
  BaseAlphaShader();

  void SetTextureSize(const base::Vec2& tex_size);
  void SetTransOffset(const base::Vec2& offset);
  void SetTexture(GLID<Texture> tex);

 private:
  GLint u_texSize_;
  GLint u_transOffset_;
  GLint u_texture_;
};

class SpriteShader : public GLES2ShaderBase {
 public:
  SpriteShader();

  void SetTextureSize(const base::Vec2& tex_size);
  void SetTransformMatrix(const float* mat4);
  void SetTexture(GLID<Texture> tex);

  void SetOpacity(float opacity);
  void SetColor(const base::Vec4& color);
  void SetTone(const base::Vec4& tone);

  void SetBushDepth(float depth);
  void SetBushOpacity(float depthOpacity);

 private:
  GLint u_texSize_;
  GLint u_transformMat_;
  GLint u_texture_;

  GLint u_color_;
  GLint u_tone_;
  GLint u_opacity_;

  GLint u_bushDepth_;
  GLint u_bushOpacity_;
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

class PlaneShader : public GLES2ShaderBase {
 public:
  PlaneShader();

  void SetTransOffset(const base::Vec2& offset);
  void SetTexture(GLID<Texture> tex);
  void SetTextureSize(const base::Vec2& tex_size);

  void SetOpacity(float opacity);
  void SetColor(const base::Vec4& color);
  void SetTone(const base::Vec4& tone);

 private:
  GLint u_transOffset_;
  GLint u_texture_;
  GLint u_texSize_;

  GLint u_opacity_;
  GLint u_color_;
  GLint u_tone_;
};

class GrayShader : public GLES2ShaderBase {
 public:
  GrayShader();

  void SetTextureSize(const base::Vec2& tex_size);
  void SetTransOffset(const base::Vec2& offset);
  void SetTexture(GLID<Texture> tex);
  void SetGray(float gray);

 private:
  GLint u_texSize_;
  GLint u_transOffset_;
  GLint u_texture_;
  GLint u_gray_;
};

class FlatShader : public GLES2ShaderBase {
 public:
  FlatShader();

  void SetColor(const base::Vec4& color);

 private:
  GLint u_color_;
};

class AlphaTransShader : public GLES2ShaderBase {
 public:
  AlphaTransShader();

  void SetTextureSize(const base::Vec2& tex_size);
  void SetTransOffset(const base::Vec2& offset);
  void SetFrozenTexture(GLID<Texture> tex);
  void SetCurrentTexture(GLID<Texture> tex);
  void SetProgress(float progress);

 private:
  GLint u_texSize_;
  GLint u_transOffset_;

  GLint u_frozen_texture_;
  GLint u_current_texture_;
  GLint u_progress_;
};

class VagueTransShader : public GLES2ShaderBase {
 public:
  VagueTransShader();

  void SetTextureSize(const base::Vec2& tex_size);
  void SetTransOffset(const base::Vec2& offset);
  void SetFrozenTexture(GLID<Texture> tex);
  void SetCurrentTexture(GLID<Texture> tex);
  void SetTransTexture(GLID<Texture> tex);
  void SetProgress(float progress);
  void SetVague(float vague);

 private:
  GLint u_texSize_;
  GLint u_transOffset_;

  GLint u_frozen_texture_;
  GLint u_current_texture_;
  GLint u_trans_texture_;
  GLint u_progress_;
  GLint u_vague_;
};

}  // namespace renderer

#endif  // !RENDERER_SHADER_GLES2_SHADERS_H_
