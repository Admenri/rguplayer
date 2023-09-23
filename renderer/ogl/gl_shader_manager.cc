#include "renderer/ogl/gl_shader_manager.h"

#include <vector>

#include "base/debug/debugwriter.h"
#include "base/exceptions/exception.h"

namespace {

static void PrintShaderLog(std::shared_ptr<gpu::GLES2CommandContext> context,
                           GLuint shader) {
  GLint logLength;
  context->glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

  std::string log(logLength, '\0');
  context->glGetShaderInfoLog(shader, static_cast<GLsizei>(log.size()), 0,
                              &log[0]);

  base::Debug() << "[Core] Shader log:\n" << log;
}

static void PrintProgramLog(std::shared_ptr<gpu::GLES2CommandContext> context,
                            GLuint program) {
  GLint logLength;
  context->glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

  std::string log(logLength, '\0');
  context->glGetProgramInfoLog(program, static_cast<GLsizei>(log.size()), 0,
                               &log[0]);

  std::clog << "[Core] Program log:\n" << log;
}

}  // namespace

namespace renderer {

namespace shader {

#include "gpu/shader/drawable.frag.xxd"
#include "gpu/shader/drawable.vert.xxd"

}  // namespace shader

GLShaderManager::GLShaderManager(std::shared_ptr<GLCC> context) : cc_(context) {
  vertex_shader_ = GetContext()->glCreateShader(GL_VERTEX_SHADER);
  frag_shader_ = GetContext()->glCreateShader(GL_FRAGMENT_SHADER);
  program_ = GetContext()->glCreateProgram();
}

GLShaderManager::~GLShaderManager() {
  GetContext()->glDeleteProgram(program_);
  GetContext()->glDeleteShader(vertex_shader_);
  GetContext()->glDeleteShader(frag_shader_);
}

void GLShaderManager::Bind() {
  if (cc_->Program().Current() == program_) return;
  cc_->Program().Set(program_);
}

void GLShaderManager::Unbind() { cc_->Program().Set(0); }

GLuint GLShaderManager::GetProgram() { return program_; }

void GLShaderManager::Setup(const std::string& vertex_shader,
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

void GLShaderManager::CompileShader(GLuint gl_shader,
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
  shader_sizes.push_back(shader_source.size());

  // Setup shader program
  GetContext()->glShaderSource(gl_shader, shader_srcs.size(),
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

GLShaderBase::GLShaderBase(std::shared_ptr<GLCC> context)
    : GLShaderManager(context), viewp_matrix_location_(0) {}

void GLShaderBase::Setup(const std::string& vertex_shader,
                         const std::string& frag_shader) {
  GLShaderManager::Setup(vertex_shader, frag_shader);

  GetContext()->glBindAttribLocation(GetProgram(), ShaderLocation::Position,
                                     "position");
  GetContext()->glBindAttribLocation(GetProgram(), ShaderLocation::TexCoord,
                                     "texCoord");
  GetContext()->glBindAttribLocation(GetProgram(), ShaderLocation::Color,
                                     "color");

  viewp_matrix_location_ =
      GetContext()->glGetUniformLocation(GetProgram(), "viewpMat");
}

void GLShaderBase::SetViewportMatrix(uint32_t w, uint32_t h) {
  const float a = 2.f / w;
  const float b = 2.f / h;
  const float c = -2.f;
  GLfloat mat[16] = {a, 0, 0, 0, 0, b, 0, 0, 0, 0, c, 0, -1, -1, -1, 1};

  GetContext()->glUniformMatrix4fv(viewp_matrix_location_, 1, GL_FALSE, mat);
}

}  // namespace renderer
