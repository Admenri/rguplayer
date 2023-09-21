#include "renderer/compositor/gl_cc.h"

namespace renderer {

GLCC::GLCC(const SDL_GLContext& gl_ctx) : gl_sdl_ctx_(gl_ctx) {
  context_ = std::make_shared<gpu::GLES2CommandContext>();

  quad_indices_buffer_ = std::make_shared<QuadIndicesBuffer>(context_);

  gl_viewport_ = std::make_unique<GLViewport>(context_);
  gl_scissor_region_ = std::make_unique<GLScissorRegion>(context_);
  gl_scissor_test_ = std::make_unique<GLScissorTest>(context_);
  gl_blend_mode_ = std::make_unique<GLBlendMode>(context_);
  gl_program_ = std::make_unique<GLProgram>(context_);
}

}  // namespace renderer
