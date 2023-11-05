// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/script/graphics.h"

#include <SDL_timer.h>

#include "content/scheduler/worker_cc.h"

namespace content {

Graphics::Graphics(base::WeakPtr<ui::Widget> window,
                   const base::Vec2i& initial_resolution)
    : window_(window), resolution_(initial_resolution) {
  viewport_rect().rect = initial_resolution;

  BindingRunner::Get()->GetRenderer()->PostTask(base::BindOnce(
      &Graphics::InitScreenBufferInternal, weak_ptr_factory_.GetWeakPtr()));
}

Graphics::~Graphics() {
  BindingRunner::Get()->GetRenderer()->PostTask(base::BindOnce(
      &Graphics::DestroyBufferInternal, weak_ptr_factory_.GetWeakPtr()));
}

void Graphics::Update() {
  bool complete_flag = false;
  BindingRunner::Get()->GetRenderer()->PostTask(
      base::BindOnce(&Graphics::PresentScreenInternal,
                     weak_ptr_factory_.GetWeakPtr(), &complete_flag));

  /* Delay for desire frame rate */
  SDL_Delay(1000 / frame_rate_);

  /* If not complete drawing */
  if (!complete_flag) {
    /* Drawtime > expect drawtime, sync for draw complete */
    BindingRunner::Get()->GetRenderer()->WaitForSync();
  }

  /* Increase frame render count */
  ++frame_count_;
}

void Graphics::InitScreenBufferInternal() {
  screen_buffer_[0] = gpu::TextureFrameBuffer::Gen();
  gpu::TextureFrameBuffer::Alloc(screen_buffer_[0], resolution_.x,
                                 resolution_.y);
  gpu::TextureFrameBuffer::LinkFrameBuffer(screen_buffer_[0]);

  screen_buffer_[1] = gpu::TextureFrameBuffer::Gen();
  gpu::TextureFrameBuffer::Alloc(screen_buffer_[1], resolution_.x,
                                 resolution_.y);
  gpu::TextureFrameBuffer::LinkFrameBuffer(screen_buffer_[1]);

  screen_quad_ = std::make_unique<gpu::QuadDrawable>();
}

void Graphics::DestroyBufferInternal() {
  gpu::TextureFrameBuffer::Del(screen_buffer_[0]);
  gpu::TextureFrameBuffer::Del(screen_buffer_[1]);

  screen_quad_.reset();
}

void Graphics::CompositeScreenInternal() {
  /* Prepare composite notify */
  DrawableParent::NotifyPrepareComposite();

  gpu::FrameBuffer::Bind(screen_buffer_[0].fbo);
  gpu::GSM.states.clear_color.Set(base::Vec4());
  gpu::FrameBuffer::Clear();

  gpu::GSM.states.viewport.Push(resolution_);
  DrawableParent::CompositeChildren();
  gpu::GSM.states.viewport.Pop();
}

void Graphics::ResizeResolutionInternal() {
  gpu::TextureFrameBuffer::Alloc(screen_buffer_[0], resolution_.x,
                                 resolution_.y);
  gpu::TextureFrameBuffer::Alloc(screen_buffer_[1], resolution_.x,
                                 resolution_.y);

  screen_quad_->SetPositionRect(base::Vec2(resolution_));
  screen_quad_->SetTexCoordRect(base::Vec2(resolution_));

  viewport_rect().rect = resolution_;
  NotifyViewportChanged();
}

void Graphics::PresentScreenInternal(bool* paint_raiser) {
  CompositeScreenInternal();

  gpu::Blt::BeginScreen(resolution_);
  gpu::Blt::TexSource(screen_buffer_[0]);

  gpu::GSM.states.clear_color.Set(base::Vec4());
  gpu::FrameBuffer::Clear();

  base::Rect target_rect(0, resolution_.y, resolution_.x, -resolution_.y);
  gpu::Blt::EndDraw(resolution_, target_rect);

  SDL_GL_SwapWindow(window_->AsSDLWindow());
  *paint_raiser = true;
}

}  // namespace content
