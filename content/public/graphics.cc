// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/graphics.h"

#include "renderer/quad/quad_drawable.h"

#include "SDL_timer.h"

namespace content {

Graphics::Graphics(scoped_refptr<RenderRunner> renderer,
                   const base::Vec2i& initial_resolution)
    : renderer_(renderer), resolution_(initial_resolution) {
  viewport_rect().rect = initial_resolution;

  renderer_->PostTask(base::BindOnce(&Graphics::InitScreenBufferInternal,
                                     weak_ptr_factory_.GetWeakPtr()));
}

Graphics::~Graphics() {
  renderer()->PostTask(base::BindOnce(&Graphics::DestroyBufferInternal,
                                      weak_ptr_factory_.GetWeakPtr()));
}

void Graphics::Update() {
  // TODO: fps manager required

  bool complete_flag = false;
  renderer()->PostTask(base::BindOnce(&Graphics::PresentScreenInternal,
                                      weak_ptr_factory_.GetWeakPtr(),
                                      &complete_flag));

  /* Delay for desire frame rate */
  SDL_Delay(1000 / 60);

  /* If not complete drawing */
  if (!complete_flag) {
    /* Drawtime > expect drawtime, sync for draw complete */
    renderer()->WaitForSync();
  }

  /* Increase frame render count */
  ++frame_count_;
}

void Graphics::InitScreenBufferInternal() {
  screen_buffer_[0] = renderer::TextureFrameBuffer::Gen();
  renderer::TextureFrameBuffer::Alloc(screen_buffer_[0], resolution_.x,
                                      resolution_.y);
  renderer::TextureFrameBuffer::LinkFrameBuffer(screen_buffer_[0]);

  screen_buffer_[1] = renderer::TextureFrameBuffer::Gen();
  renderer::TextureFrameBuffer::Alloc(screen_buffer_[1], resolution_.x,
                                      resolution_.y);
  renderer::TextureFrameBuffer::LinkFrameBuffer(screen_buffer_[1]);

  screen_quad_ = std::make_unique<renderer::QuadDrawable>();
}

void Graphics::DestroyBufferInternal() {
  renderer::TextureFrameBuffer::Del(screen_buffer_[0]);
  renderer::TextureFrameBuffer::Del(screen_buffer_[1]);

  screen_quad_.reset();
}

void Graphics::CompositeScreenInternal() {
  /* Prepare composite notify */
  DrawableParent::NotifyPrepareComposite();

  renderer::FrameBuffer::Bind(screen_buffer_[0].fbo);
  renderer::GSM.states.clear_color.Set(base::Vec4());
  renderer::FrameBuffer::Clear();

  renderer::GSM.states.scissor_rect.Set(resolution_);
  renderer::GSM.states.viewport.Push(resolution_);
  DrawableParent::CompositeChildren();
  renderer::GSM.states.viewport.Pop();
}

void Graphics::ResizeResolutionInternal() {
  renderer::TextureFrameBuffer::Alloc(screen_buffer_[0], resolution_.x,
                                      resolution_.y);
  renderer::TextureFrameBuffer::Alloc(screen_buffer_[1], resolution_.x,
                                      resolution_.y);

  screen_quad_->SetPositionRect(base::Vec2(resolution_));
  screen_quad_->SetTexCoordRect(base::Vec2(resolution_));

  viewport_rect().rect = resolution_;
  NotifyViewportChanged();
}

void Graphics::PresentScreenInternal(bool* paint_raiser) {
  CompositeScreenInternal();

  renderer::Blt::BeginScreen(resolution_);
  renderer::Blt::TexSource(screen_buffer_[0]);

  renderer::GSM.states.clear_color.Set(base::Vec4());
  renderer::FrameBuffer::Clear();

  base::Rect target_rect(0, resolution_.y, resolution_.x, -resolution_.y);
  renderer::Blt::EndDraw(resolution_, target_rect);

  SDL_GL_SwapWindow(renderer_->window());
  *paint_raiser = true;
}

}  // namespace content
