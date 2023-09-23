#ifndef RENDERER_OGL_GL_SHADER_MANAGER_H_
#define RENDERER_OGL_GL_SHADER_MANAGER_H_

#include <memory>

#include "base/math/math.h"
#include "gpu/gles2/gles_context.h"
#include "renderer/compositor/gl_cc.h"

namespace renderer {

class GLShaderManager {
 public:
  GLShaderManager(std::shared_ptr<GLCC> cc);
  virtual ~GLShaderManager();

  GLShaderManager(const GLShaderManager&) = delete;
  GLShaderManager& operator=(const GLShaderManager&) = delete;

  void Bind();
  void Unbind();
  GLuint GetProgram();

 protected:
  void Setup(const std::string& vertex_shader, const std::string& frag_shader);
  void CompileShader(GLuint gl_shader, const std::string& shader_source);

  std::shared_ptr<gpu::GLES2CommandContext> GetContext() {
    return cc_->GetContext();
  }

 private:
  std::shared_ptr<GLCC> cc_;
  GLuint vertex_shader_;
  GLuint frag_shader_;
  GLuint program_;
};

class GLShaderBase : public GLShaderManager {
 public:
  GLShaderBase(std::shared_ptr<GLCC> context);

  void Setup(const std::string& vertex_shader, const std::string& frag_shader);
  void SetViewportMatrix(uint32_t w, uint32_t h);

 private:
  GLint viewp_matrix_location_;
};

}  // namespace renderer

#endif  // RENDERER_OGL_GL_SHADER_MANAGER_H_