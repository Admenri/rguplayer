// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/gles2/shader/shader_manager.h"

#include <vector>

#include "base/debug/debugwriter.h"
#include "base/exceptions/exception.h"

namespace {

static void PrintShaderLog(scoped_refptr<gpu::GLES2CommandContext> context,
                           GLuint shader) {
  GLint logLength;
  context->glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

  std::string log(logLength, '\0');
  context->glGetShaderInfoLog(shader, static_cast<GLsizei>(log.size()), 0,
                              &log[0]);

  base::Debug() << "[Core] Shader log:\n" << log;
}

static void PrintProgramLog(scoped_refptr<gpu::GLES2CommandContext> context,
                            GLuint program) {
  GLint logLength;
  context->glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

  std::string log(logLength, '\0');
  context->glGetProgramInfoLog(program, static_cast<GLsizei>(log.size()), 0,
                               &log[0]);

  std::clog << "[Core] Program log:\n" << log;
}

}  // namespace

namespace gpu {

namespace shader {

#include "gpu/gles2/shader/shader_source/base.frag.xxd"
#include "gpu/gles2/shader/shader_source/base.vert.xxd"
#include "gpu/gles2/shader/shader_source/drawable.vert.xxd"
#include "gpu/gles2/shader/shader_source/textureblt.frag.xxd"

static inline std::string FromRawData(const uint8_t* raw_data,
                                      const uint32_t data_size) {
  return std::string(reinterpret_cast<const char*>(raw_data), data_size);
}

}  // namespace shader

ShaderManager::ShaderManager(scoped_refptr<gpu::GLES2CommandContext> context)
    : context_(context) {
  vertex_shader_ = GetContext()->glCreateShader(GL_VERTEX_SHADER);
  frag_shader_ = GetContext()->glCreateShader(GL_FRAGMENT_SHADER);
  program_ = GetContext()->glCreateProgram();
}

ShaderManager::~ShaderManager() {
  GetContext()->glDeleteProgram(program_);
  GetContext()->glDeleteShader(vertex_shader_);
  GetContext()->glDeleteShader(frag_shader_);
}

void ShaderManager::Bind() { GetContext()->glUseProgram(program_); }

void ShaderManager::Unbind() { GetContext()->glUseProgram(0); }

GLuint ShaderManager::GetProgram() { return program_; }

void ShaderManager::Setup(const std::string& vertex_shader,
                          const std::string& frag_shader) {
  CompileShader(vertex_shader_, vertex_shader);
  CompileShader(frag_shader_, frag_shader);

  GetContext()->glAttachShader(program_, vertex_shader_);
  GetContext()->glAttachShader(program_, frag_shader_);

  // Bind attribute
  GetContext()->glBindAttribLocation(GetProgram(), ShaderLocation::Position,
                                     "position");
  GetContext()->glBindAttribLocation(GetProgram(), ShaderLocation::TexCoord,
                                     "texCoord");
  GetContext()->glBindAttribLocation(GetProgram(), ShaderLocation::Color,
                                     "color");

  // Before link bind
  GetContext()->glLinkProgram(program_);

  GLint success;
  GetContext()->glGetProgramiv(program_, GL_LINK_STATUS, &success);
  if (!success) {
    PrintProgramLog(GetContext(), program_);
    throw base::Exception(base::Exception::OpenGLError,
                          "GLSL: An error occured while linking program.");
  }
}

void ShaderManager::CompileShader(GLuint gl_shader,
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
  GetContext()->glShaderSource(gl_shader,
                               static_cast<GLsizei>(shader_srcs.size()),
                               shader_srcs.data(), shader_sizes.data());

  GetContext()->glCompileShader(gl_shader);
  GLint success;
  GetContext()->glGetShaderiv(gl_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    PrintShaderLog(GetContext(), gl_shader);
    throw base::Exception(base::Exception::OpenGLError,
                          "GLSL: An error occured while compiling shader.");
  }
}

ShaderBase::ShaderBase(scoped_refptr<gpu::GLES2CommandContext> context)
    : ShaderManager(context), viewp_matrix_location_(0) {}

void ShaderBase::Setup(const std::string& vertex_shader,
                       const std::string& frag_shader) {
  ShaderManager::Setup(vertex_shader, frag_shader);

  viewp_matrix_location_ =
      GetContext()->glGetUniformLocation(GetProgram(), "viewpMat");
}

void ShaderBase::SetTexture(GLint location, GLuint tex, uint16_t unit) {
  GetContext()->glActiveTexture(GL_TEXTURE0 + unit);
  GetContext()->glBindTexture(GL_TEXTURE_2D, tex);
  GetContext()->glUniform1i(location, unit);
  GetContext()->glActiveTexture(GL_TEXTURE0);
}

void ShaderBase::SetViewportMatrix(const base::Vec2i& size) {
  const float a = 2.f / size.x;
  const float b = 2.f / size.y;
  const float c = -2.f;
  GLfloat mat[16] = {a, 0, 0, 0, 0, b, 0, 0, 0, 0, c, 0, -1, -1, -1, 1};

  GetContext()->glUniformMatrix4fv(viewp_matrix_location_, 1, GL_FALSE, mat);
}

DrawableShader::DrawableShader(scoped_refptr<gpu::GLES2CommandContext> context)
    : ShaderBase(context) {
  ShaderBase::Setup(
      shader::FromRawData(shader::drawable_vert, shader::drawable_vert_len),
      shader::FromRawData(shader::base_frag, shader::base_frag_len));

  tex_size_location_ =
      GetContext()->glGetUniformLocation(GetProgram(), "texSize");
  transform_matrix_location_ =
      GetContext()->glGetUniformLocation(GetProgram(), "transformMat");
  texture_location_ =
      GetContext()->glGetUniformLocation(GetProgram(), "texture");
}

void DrawableShader::SetTextureSize(const base::Vec2& tex_size) {
  GetContext()->glUniform2f(tex_size_location_, 1.f / tex_size.x,
                            1.f / tex_size.y);
}

void DrawableShader::SetTransformMatrix(const float* transform) {
  GetContext()->glUniformMatrix4fv(transform_matrix_location_, 1, GL_FALSE,
                                   transform);
}

void DrawableShader::SetTexture(GLuint tex) {
  ShaderBase::SetTexture(texture_location_, tex, 1);
}

BaseShader::BaseShader(scoped_refptr<gpu::GLES2CommandContext> context)
    : ShaderBase(context) {
  ShaderBase::Setup(
      shader::FromRawData(shader::base_vert, shader::base_vert_len),
      shader::FromRawData(shader::base_frag, shader::base_frag_len));

  tex_size_location_ =
      GetContext()->glGetUniformLocation(GetProgram(), "texSize");
  trans_offset_location_ =
      GetContext()->glGetUniformLocation(GetProgram(), "transOffset");
  texture_location_ =
      GetContext()->glGetUniformLocation(GetProgram(), "texture");
}

void BaseShader::SetTextureSize(const base::Vec2& tex_size) {
  GetContext()->glUniform2f(tex_size_location_, 1.f / tex_size.x,
                            1.f / tex_size.y);
}

void BaseShader::SetTransOffset(const base::Vec2& offset) {
  GetContext()->glUniform2f(trans_offset_location_, offset.x, offset.y);
}

void BaseShader::SetTexture(GLuint tex) {
  ShaderBase::SetTexture(texture_location_, tex, 1);
}

BltShader::BltShader(scoped_refptr<gpu::GLES2CommandContext> context)
    : ShaderBase(context) {
  ShaderBase::Setup(
      shader::FromRawData(shader::base_vert, shader::base_vert_len),
      shader::FromRawData(shader::textureblt_frag,
                          shader::textureblt_frag_len));

  tex_size_location_ =
      GetContext()->glGetUniformLocation(GetProgram(), "texSize");
  trans_offset_location_ =
      GetContext()->glGetUniformLocation(GetProgram(), "transOffset");

  src_texture_location_ =
      GetContext()->glGetUniformLocation(GetProgram(), "src_texture");
  dst_texture_location_ =
      GetContext()->glGetUniformLocation(GetProgram(), "dst_texture");

  sub_rect_location_ =
      GetContext()->glGetUniformLocation(GetProgram(), "subRect");
  opacity_location_ =
      GetContext()->glGetUniformLocation(GetProgram(), "opacity");
}

void BltShader::SetTexture(GLuint tex) {
  ShaderBase::SetTexture(src_texture_location_, tex, 1);
}

void BltShader::SetTextureSize(const base::Vec2& tex_size) {
  GetContext()->glUniform2f(tex_size_location_, 1.f / tex_size.x,
                            1.f / tex_size.y);
}

void BltShader::SetTransOffset(const base::Vec2& offset) {
  GetContext()->glUniform2f(trans_offset_location_, offset.x, offset.y);
}

void BltShader::SetDstTexture(GLuint tex) {
  ShaderBase::SetTexture(dst_texture_location_, tex, 2);
}

void BltShader::SetOpacity(float opacity) {
  GetContext()->glUniform1f(opacity_location_, opacity);
}

void BltShader::SetSubRect(const base::Vec4& rect) {
  GetContext()->glUniform4f(sub_rect_location_, rect.x, rect.y, rect.z, rect.w);
}

}  // namespace gpu
