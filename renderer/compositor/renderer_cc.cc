// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "renderer/compositor/renderer_cc.h"

namespace renderer {

CCLayer::CCLayer(base::WeakPtr<ui::Widget> window, const SDL_GLContext& gl_ctx)
    : gl_sdl_ctx_(gl_ctx), window_(window), texture_max_size_(0) {
  context_ = base::MakeRefCounted<gpu::GLES2CommandContext>();
  context_->InitContext();

  /* Init common indices object */
  quad_indices_buffer_ = base::MakeRefCounted<QuadIndicesBuffer>(context_);

  /* Init renderer state */
  states_.viewport = std::make_unique<GLViewport>(context_);
  states_.scissor_region = std::make_unique<GLScissorRegion>(context_);
  states_.scissor_test = std::make_unique<GLScissorTest>(context_);
  states_.blend = std::make_unique<GLBlend>(context_);
  states_.blend_mode = std::make_unique<GLBlendMode>(context_);

  /* Init caps */
  context_->glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texture_max_size_);

  /* Init shaders */
  shaders_.base_shader = std::make_unique<gpu::BaseShader>(context_);
  shaders_.drawable_shader = std::make_unique<gpu::DrawableShader>(context_);

  /* Init viewport and clear */
  States()->viewport->Push(base::Rect(base::Vec2i(), window->GetSize()));
  States()->blend->Push(true);
  States()->blend_mode->Push(renderer::BlendMode::Normal);
  States()->scissor_test->Push(false);
  States()->scissor_region->Push(base::Rect());

  GetContext()->glClearColor(0, 0, 0, 0);
  GetContext()->glClear(GL_COLOR_BUFFER_BIT);
  SDL_GL_SwapWindow(window->AsSDLWindow());
}

}  // namespace renderer
