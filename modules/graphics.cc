// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "modules/graphics.h"

namespace modules {

Screen::Screen(scoped_refptr<content::RendererThread> render_thread)
    : render_thread_(render_thread) {
  render_thread_->GetRenderThreadRunner()->PostTask(base::BindOnce(
      &Screen::InitScreenInternal, weak_ptr_factory_.GetWeakPtr()));
}

Screen::~Screen() {
  render_thread_->GetRenderThreadRunner()->DeleteSoon(std::move(screen_quad_));
  render_thread_->GetRenderThreadRunner()->DeleteSoon(
      std::move(double_buffer_));
}

void Screen::Composite() {
  base::RunLoop sync_loop;
  render_thread_->GetRenderThreadRunner()->PostTask(
      base::BindOnce(&Screen::CompositeInternal, weak_ptr_factory_.GetWeakPtr(),
                     sync_loop.QuitClosure()));
  sync_loop.Run();
}

void Screen::ResizeScreen(const base::Vec2i& size) {
  render_thread_->GetRenderThreadRunner()->PostTask(base::BindOnce(
      &Screen::ResizeScreenInternal, weak_ptr_factory_.GetWeakPtr(), size));
}

void Screen::InitScreenInternal() {
  auto* cc = content::RendererThread::GetCCForRenderer();

  screen_quad_.reset(
      new renderer::QuadDrawable(cc->GetQuadIndicesBuffer(), cc->GetContext()));
  double_buffer_.reset(new renderer::DoubleFrameBuffer(
      cc->GetContext(), cc->GetWindow()->GetSize()));
}

void Screen::CompositeInternal(base::OnceClosure sync_complete) {
  auto* cc = content::RendererThread::GetCCForRenderer();

  cc->States()->viewport->Push(viewport_.rect_);
  double_buffer_->GetFrontend()->frame_buffer->Bind();

  DrawablesPaint();

  double_buffer_->GetFrontend()->frame_buffer->Unbind();
  cc->States()->viewport->Pop();

  std::move(sync_complete).Run();
}

void Screen::ResizeScreenInternal(const base::Vec2i& size) {
  double_buffer_->Resize(size);

  viewport_.rect_ = base::Rect(base::Vec2i(), size);

  NotifyViewportChanged();
}

void Screen::SwapBuffer(renderer::CCLayer* cc) {
  SDL_GL_SwapWindow(cc->GetWindow()->AsSDLWindow());
}

Graphics::Graphics(scoped_refptr<content::RendererThread> render_thread) {
  screen_.reset(new Screen(render_thread));
}

Graphics::~Graphics() {}

int Graphics::GetWidth() const { return screen_info_.resolution.x; }

int Graphics::GetHeight() const { return screen_info_.resolution.y; }

void Graphics::Update() {
  // Render to screen backend buffer
  GetScreen()->Composite();

  // Present to screen
  render_thread_->GetRenderThreadRunner()->PostTask(base::BindOnce(
      &Graphics::DrawScreenInternal, weak_ptr_factory_.GetWeakPtr()));
}

void Graphics::DrawScreenInternal() {
  auto* cc = content::RendererThread::GetCCForRenderer();

  renderer::DoubleFrameBuffer* frame_buffer = screen_->GetScreenBuffer();

  base::Rect screen_rect(base::Vec2i(), screen_info_.resolution);

  renderer::GLFrameBuffer::BltBegin(cc, nullptr, screen_rect.Size());
  renderer::GLFrameBuffer::BltSource(cc, frame_buffer->GetFrontend()->texture);
  renderer::GLFrameBuffer::BltEnd(
      cc, nullptr, screen_rect,
      base::Rect(screen_info_.display_offset_.x,
                 screen_info_.display_offset_.y + screen_rect.height,
                 screen_rect.width, -screen_rect.height));
}

}  // namespace modules
