#ifndef RENDERER_OGL_GL_SHADER_MANAGER_H_
#define RENDERER_OGL_GL_SHADER_MANAGER_H_

#include <memory>

#include "gpu/gles2/gles_context.h"

namespace renderer {

class GLShaderManager {
 public:
  GLShaderManager(std::shared_ptr<gpu::GLES2CommandContext> context);
  virtual ~GLShaderManager();

  GLShaderManager(const GLShaderManager&) = delete;
  GLShaderManager& operator=(const GLShaderManager&) = delete;

 protected:
  void Setup(const std::string& vertex_shader, const std::string& frag_shader);
  void CompileShader(GLuint gl_shader, const std::string& shader_source);

  std::shared_ptr<gpu::GLES2CommandContext> GetContext() { return context_; }

 private:
  std::shared_ptr<gpu::GLES2CommandContext> context_;
  GLuint vertex_shader_;
  GLuint frag_shader_;
  GLuint program_;
};

}  // namespace renderer

#endif  // RENDERER_OGL_GL_SHADER_MANAGER_H_