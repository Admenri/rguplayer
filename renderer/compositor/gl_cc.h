#ifndef RENDERER_COMPOSITOR_GL_CC_H_
#define RENDERER_COMPOSITOR_GL_CC_H_

#include <SDL_video.h>

#include <memory>

#include "gpu/gles2/gles_context.h"
#include "renderer/buffer_wrapper/gl_vertex.h"
#include "renderer/ogl/gl_frame_buffer_texture.h"
#include "renderer/ogl/gl_statestack.h"
#include "renderer/quad_draw.h"

namespace renderer {

class GLCC {
 public:
  GLCC(const SDL_GLContext& gl_ctx);
  virtual ~GLCC() = default;

  GLCC(const GLCC&) = delete;
  GLCC& operator=(const GLCC&) = delete;

  std::shared_ptr<gpu::GLES2CommandContext> GetContext() { return context_; }

  std::shared_ptr<QuadIndicesBuffer> GetQuadIndicesBuffer() {
    return quad_indices_buffer_;
  }

  GLViewport& Viewport() { return *gl_viewport_; }
  GLScissorRegion& ScissorRegion() { return *gl_scissor_region_; }
  GLScissorTest& ScissorTest() { return *gl_scissor_test_; }
  GLBlendMode& BlendMode() { return *gl_blend_mode_; }
  GLProgram& Program() { return *gl_program_; }

 private:
  std::shared_ptr<gpu::GLES2CommandContext> context_;

  SDL_GLContext gl_sdl_ctx_;

  std::shared_ptr<QuadIndicesBuffer> quad_indices_buffer_;

  std::unique_ptr<GLViewport> gl_viewport_;
  std::unique_ptr<GLScissorRegion> gl_scissor_region_;
  std::unique_ptr<GLScissorTest> gl_scissor_test_;
  std::unique_ptr<GLBlendMode> gl_blend_mode_;
  std::unique_ptr<GLProgram> gl_program_;
};

}  // namespace renderer

#endif  // RENDERER_COMPOSITOR_GL_CC_H_