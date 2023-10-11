// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/gles2/shader/shader_manager.h"

#include <vector>

#include "base/debug/debugwriter.h"
#include "base/exceptions/exception.h"
#include "gpu/gles2/gsm/gles_gsm.h"

namespace gpu {

namespace shader {

#include "gpu/gles2/shader/shader_source/base.frag.xxd"
#include "gpu/gles2/shader/shader_source/base.vert.xxd"

static inline std::string FromRawData(const uint8_t* raw_data,
                                      const uint32_t data_size) {
  return std::string(reinterpret_cast<const char*>(raw_data), data_size);
}

}  // namespace shader

GLES2Shader::GLES2Shader() {
  vertex_shader_ = GL.CreateShader(GL_VERTEX_SHADER);
  frag_shader_ = GL.CreateShader(GL_FRAGMENT_SHADER);
  program_ = GL.CreateProgram();
}

GLES2Shader::~GLES2Shader() {
  GL.DeleteProgram(program_);
  GL.DeleteShader(vertex_shader_);
  GL.DeleteShader(frag_shader_);
}

void GLES2Shader::Bind() { GSM.program.Set(program_); }

void GLES2Shader::Unbind() {
  GSM.program.Set(0);
  GL.ActiveTexture(GL_TEXTURE0);
}

void GLES2Shader::Setup(const std::string& vertex_shader,
                        const std::string& frag_shader) {
  CompileShader(vertex_shader_, vertex_shader);
  CompileShader(frag_shader_, frag_shader);

  GL.AttachShader(program_, vertex_shader_);
  GL.AttachShader(program_, frag_shader_);

  // Bind attribute
  if (!BindAttribLocation()) {
    GL.BindAttribLocation(program_, ShaderAttribLocation::Position,
                          "a_position");
    GL.BindAttribLocation(program_, ShaderAttribLocation::TexCoord,
                          "a_texCoord");
    GL.BindAttribLocation(program_, ShaderAttribLocation::Color, "a_color");
  }

  // Before link bind
  GL.LinkProgram(program_);

  GLint success;
  GL.GetProgramiv(program_, GL_LINK_STATUS, &success);
  if (!success) {
    GLint log_length;
    GL.GetProgramiv(program_, GL_INFO_LOG_LENGTH, &log_length);

    std::string log(log_length, '\0');
    GL.GetProgramInfoLog(program_, static_cast<GLsizei>(log.size()), 0,
                         &log[0]);

    base::Debug() << "[GLSL]" << log;

    throw base::Exception(base::Exception::OpenGLError,
                          "GLSL: An error occured while linking program.");
  }
}

void GLES2Shader::CompileShader(GLuint gl_shader,
                                const std::string& shader_source) {
  /*
    Vertex shader:
      0. Version Defines
      1. Common Header
      2. Shader body
  */

  std::vector<const GLchar*> shader_srcs;
  std::vector<GLint> shader_sizes;

  // Setup shader source
  shader_srcs.push_back(reinterpret_cast<const GLchar*>(shader_source.c_str()));
  shader_sizes.push_back(static_cast<GLint>(shader_source.size()));

  // Setup shader program
  GL.ShaderSource(gl_shader, static_cast<GLsizei>(shader_srcs.size()),
                  shader_srcs.data(), shader_sizes.data());

  GL.CompileShader(gl_shader);
  GLint success;
  GL.GetShaderiv(gl_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    GLint log_length;
    GL.GetShaderiv(gl_shader, GL_INFO_LOG_LENGTH, &log_length);

    std::string log(log_length, '\0');
    GL.GetShaderInfoLog(gl_shader, static_cast<GLsizei>(log.size()), 0,
                        &log[0]);

    base::Debug() << "[GLSL]" << log;

    throw base::Exception(base::Exception::OpenGLError,
                          "GLSL: An error occured while compiling shader.");
  }
}

GLES2ShaderBase::GLES2ShaderBase() : u_projectionMat_(0) {}

void GLES2ShaderBase::Setup(const std::string& vertex_shader,
                            const std::string& frag_shader) {
  GLES2Shader::Setup(vertex_shader, frag_shader);

  u_projectionMat_ = GL.GetUniformLocation(program_, "u_projectionMat");
}

void GLES2ShaderBase::SetTexture(GLint location, GLuint tex, uint16_t unit) {
  GL.ActiveTexture(GL_TEXTURE0 + unit);
  GL.BindTexture(GL_TEXTURE_2D, tex);
  GL.Uniform1i(location, unit);
  GL.ActiveTexture(GL_TEXTURE0);
}

void GLES2ShaderBase::SetProjectionMatrix(const base::Vec2i& size) {
  const float a = 2.f / size.x;
  const float b = 2.f / size.y;
  const float c = -2.f;
  GLfloat mat[16] = {a, 0, 0, 0, 0, b, 0, 0, 0, 0, c, 0, -1, -1, -1, 1};

  GL.UniformMatrix4fv(u_projectionMat_, 1, GL_FALSE, mat);
}

BaseShader::BaseShader() {
  GLES2ShaderBase::Setup(
      shader::FromRawData(shader::base_vert, shader::base_vert_len),
      shader::FromRawData(shader::base_frag, shader::base_frag_len));

  u_texSize_ = GL.GetUniformLocation(program_, "u_texSize");
  u_transOffset_ = GL.GetUniformLocation(program_, "u_transOffset");
  u_texture_ = GL.GetUniformLocation(program_, "u_texture");
}

void BaseShader::SetTextureSize(const base::Vec2& tex_size) {
  GL.Uniform2f(u_texSize_, 1.f / tex_size.x, 1.f / tex_size.y);
}

void BaseShader::SetTransOffset(const base::Vec2& offset) {
  GL.Uniform2f(u_transOffset_, offset.x, offset.y);
}

void BaseShader::SetTexture(GLuint tex) {
  GLES2ShaderBase::SetTexture(u_texture_, tex, 1);
}

bool BaseShader::BindAttribLocation() {
  GL.BindAttribLocation(program_, ShaderAttribLocation::Position, "a_position");
  GL.BindAttribLocation(program_, ShaderAttribLocation::TexCoord, "a_texCoord");
  return true;
}

}  // namespace gpu
