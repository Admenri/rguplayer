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
  quad_ = std::make_unique<QuadDrawable>(quad_indices_buffer_, context_);

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
  shaders_.blt_shader = std::make_unique<gpu::BltShader>(context_);
  shaders_.color_shader = std::make_unique<gpu::ColorShader>(context_);

  /* Init viewport and clear */
  States()->viewport->Push(base::Rect(base::Vec2i(), window->GetSize()));
  States()->blend->Push(true);
  States()->blend_mode->Push(renderer::BlendMode::Normal);
  States()->scissor_test->Push(false);
  States()->scissor_region->Push(base::Rect());

  /* Init reuse texture */
  reuse_texture_ = new renderer::GLTexture(context_);
  reuse_frame_buffer_.reset(new renderer::GLFrameBuffer(context_));

  reuse_texture_->Bind();
  reuse_texture_->SetSize(base::Vec2i(128, 128));
  reuse_texture_->SetTextureFilter(GL_NEAREST);
  reuse_texture_->AllocEmpty();

  reuse_frame_buffer_->Bind();
  reuse_frame_buffer_->SetRenderTarget(reuse_texture_);
  reuse_frame_buffer_->Clear();
  reuse_frame_buffer_->Unbind();

  GetContext()->glClearColor(0, 0, 0, 0);
  GetContext()->glClear(GL_COLOR_BUFFER_BIT);
  SDL_GL_SwapWindow(window->AsSDLWindow());
}

void CCLayer::ResizeReusedTextureIfNeed(const base::Vec2i& size) {
  if (reuse_texture_->GetSize().x >= size.x &&
      reuse_texture_->GetSize().y >= size.y)
    return;

  auto new_size = base::Vec2i(std::max(reuse_texture_->GetSize().x, size.x),
                              std::max(reuse_texture_->GetSize().y, size.y));

  reuse_texture_->SetSize(new_size);
  reuse_texture_->Bind();
  reuse_texture_->AllocEmpty();
}

}  // namespace renderer
