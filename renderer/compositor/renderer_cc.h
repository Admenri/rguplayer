// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RENDERER_COMPOSITOR_RENDERER_CC_H_
#define RENDERER_COMPOSITOR_RENDERER_CC_H_

#include <SDL_video.h>

#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "gpu/gl_forward.h"
#include "gpu/gles2/shader/shader_manager.h"
#include "renderer/buffer/vertex_buffer.h"
#include "renderer/paint/frame_buffer_canvas.h"
#include "renderer/paint/quad_draw.h"
#include "renderer/state/state_stack.h"

namespace renderer {

class CCLayer {
 public:
  CCLayer(const SDL_GLContext& gl_ctx);
  virtual ~CCLayer() = default;

  CCLayer(const CCLayer&) = delete;
  CCLayer& operator=(const CCLayer&) = delete;

  scoped_refptr<gpu::GLES2CommandContext> GetContext() { return context_; }

  scoped_refptr<QuadIndicesBuffer> GetQuadIndicesBuffer() {
    return quad_indices_buffer_;
  }

  GLViewport& Viewport() { return *states.viewport_; }
  GLScissorRegion& ScissorRegion() { return *states.scissor_region_; }
  GLScissorTest& ScissorTest() { return *states.scissor_test_; }
  GLBlendMode& BlendMode() { return *states.blend_mode_; }

  base::WeakPtr<CCLayer> AsWeakPtr() { return weak_ptr_factory_.GetWeakPtr(); }

  SDL_GLContext GetSDLGLCtx() { return gl_sdl_ctx_; }

 private:
  // GL FUNCTION Context for current cc layer
  scoped_refptr<gpu::GLES2CommandContext> context_;

  // Quad index buffer object for current cc layer
  scoped_refptr<QuadIndicesBuffer> quad_indices_buffer_;

  // Current thread gl context handle
  SDL_GLContext gl_sdl_ctx_;

  // Stack style state storage
  struct {
    std::unique_ptr<GLViewport> viewport_;
    std::unique_ptr<GLScissorRegion> scissor_region_;
    std::unique_ptr<GLScissorTest> scissor_test_;
    std::unique_ptr<GLBlendMode> blend_mode_;
  } states;

  // Shader object
  struct {
    std::unique_ptr<gpu::SimpleShader> simple_shader;
  } shaders;

  base::WeakPtrFactory<CCLayer> weak_ptr_factory_{this};
};

}  // namespace renderer

#endif  // RENDERER_COMPOSITOR_RENDERER_CC_H_