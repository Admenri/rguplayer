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

#include "gpu/gles2/shader/shader_source/drawable.frag.xxd"
#include "gpu/gles2/shader/shader_source/drawable.vert.xxd"

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

  GetContext()->glBindAttribLocation(GetProgram(), ShaderLocation::Position,
                                     "position");
  GetContext()->glBindAttribLocation(GetProgram(), ShaderLocation::TexCoord,
                                     "texCoord");
  GetContext()->glBindAttribLocation(GetProgram(), ShaderLocation::Color,
                                     "color");

  viewp_matrix_location_ =
      GetContext()->glGetUniformLocation(GetProgram(), "viewpMat");
}

void ShaderBase::SetViewportMatrix(const base::Vec2i& size) {
  const float a = 2.f / size.x;
  const float b = 2.f / size.y;
  const float c = -2.f;
  GLfloat mat[16] = {a, 0, 0, 0, 0, b, 0, 0, 0, 0, c, 0, -1, -1, -1, 1};

  GetContext()->glUniformMatrix4fv(viewp_matrix_location_, 1, GL_FALSE, mat);
}

SimpleShader::SimpleShader(scoped_refptr<gpu::GLES2CommandContext> context)
    : ShaderBase(context) {
  ShaderBase::Setup(
      shader::FromRawData(shader::drawable_vert, shader::drawable_vert_len),
      shader::FromRawData(shader::drawable_frag, shader::drawable_frag_len));

  tex_size_location_ =
      GetContext()->glGetUniformLocation(GetProgram(), "texSize");
  trans_offset_location_ =
      GetContext()->glGetUniformLocation(GetProgram(), "transOffset");
}

void SimpleShader::SetTextureSize(const base::Vec2& tex_size) {
  GetContext()->glUniform2f(tex_size_location_, tex_size.x, tex_size.y);
}

void SimpleShader::SetTransOffset(const base::Vec2& offset) {
  GetContext()->glUniform2f(trans_offset_location_, offset.x, offset.y);
}

}  // namespace gpu
