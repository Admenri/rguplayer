// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_GLES2_SHADER_SHADER_MANAGER_H_
#define GPU_GLES2_SHADER_SHADER_MANAGER_H_

#include "base/math/math.h"
#include "base/memory/ref_counted.h"
#include "gpu/gl_forward.h"

namespace gpu {

enum ShaderLocation : GLuint {
  Position = 0,
  TexCoord,
  Color,
};

class ShaderManager {
 public:
  ShaderManager(scoped_refptr<gpu::GLES2CommandContext> context);
  virtual ~ShaderManager();

  ShaderManager(const ShaderManager&) = delete;
  ShaderManager& operator=(const ShaderManager&) = delete;

  void Bind();
  void Unbind();
  GLuint GetProgram();

 protected:
  virtual void Setup(const std::string& vertex_shader,
                     const std::string& frag_shader);
  void CompileShader(GLuint gl_shader, const std::string& shader_source);

  scoped_refptr<gpu::GLES2CommandContext> GetContext() { return context_; }

 private:
  scoped_refptr<gpu::GLES2CommandContext> context_;

  GLuint vertex_shader_;
  GLuint frag_shader_;
  GLuint program_;
};

class ShaderBase : public ShaderManager {
 public:
  ShaderBase(scoped_refptr<gpu::GLES2CommandContext> context);

  void SetViewportMatrix(const base::Vec2i& size);

 protected:
  void Setup(const std::string& vertex_shader,
             const std::string& frag_shader) override;

  void SetTexture(GLint location, GLuint tex, uint16_t unit);

 private:
  GLint viewp_matrix_location_;
};

class BaseShader : public ShaderBase {
 public:
  BaseShader(scoped_refptr<gpu::GLES2CommandContext> context);

  void SetTextureSize(const base::Vec2& tex_size);
  void SetTransOffset(const base::Vec2& offset);
  void SetTexture(GLuint tex);

 private:
  GLint tex_size_location_;
  GLint trans_offset_location_;

  GLint texture_location_;
};

class DrawableShader : public ShaderBase {
 public:
  DrawableShader(scoped_refptr<gpu::GLES2CommandContext> context);

  void SetTextureSize(const base::Vec2& tex_size);
  void SetTransformMatrix(const float* transform);
  void SetTexture(GLuint tex);

 private:
  GLint tex_size_location_;
  GLint transform_matrix_location_;

  GLint texture_location_;
};

}  // namespace gpu

#endif  // GPU_GLES2_SHADER_SHADER_MANAGER_H_