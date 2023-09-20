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

static const char kGLESDefine[] = "#define GLSLES\n";
static const char kFragDefine[] = "#define FRAGMENT_SHADER\n";

namespace shader {

#include "gpu/shader/common_header.h.xxd"

}

GLShaderManager::GLShaderManager(
    std::shared_ptr<gpu::GLES2CommandContext> context)
    : context_(context) {
  vertex_shader_ = GetContext()->glCreateShader(GL_VERTEX_SHADER);
  frag_shader_ = GetContext()->glCreateShader(GL_FRAGMENT_SHADER);
  program_ = GetContext()->glCreateProgram();
}

GLShaderManager::~GLShaderManager() {
  GetContext()->glDeleteProgram(program_);
  GetContext()->glDeleteShader(vertex_shader_);
  GetContext()->glDeleteShader(frag_shader_);
}

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

  // Shader version define
  if (GetContext()->IsGLES()) {
    shader_srcs.push_back(kGLESDefine);
    shader_sizes.push_back(sizeof(kGLESDefine));
  }

  // Setup common header
  shader_srcs.push_back(
      reinterpret_cast<const GLchar*>(shader::common_header_h));
  shader_sizes.push_back(shader::common_header_h_len);

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

}  // namespace renderer
