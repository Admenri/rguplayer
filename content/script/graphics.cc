#include "content/script/graphics.h"

#include "content/scheduler/worker_cc.h"

namespace content {

Graphics::Graphics(base::WeakPtr<ui::Widget> window,
                   const base::Vec2i& initial_resolution)
    : window_(window), resolution_(initial_resolution) {
  WorkerTreeHost::GetInstance()->GetRenderTaskRunner()->PostTask(base::BindOnce(
      &Graphics::InitScreenBufferInternal, weak_ptr_factory_.GetWeakPtr()));
}

Graphics::~Graphics() {
  WorkerTreeHost::GetInstance()->GetRenderTaskRunner()->PostTask(base::BindOnce(
      &Graphics::DestroyBufferInternal, weak_ptr_factory_.GetWeakPtr()));
}

void Graphics::Update() {
  WorkerTreeHost::GetInstance()->GetRenderTaskRunner()->PostTask(base::BindOnce(
      &Graphics::PresentScreenInternal, weak_ptr_factory_.GetWeakPtr()));
  WorkerTreeHost::GetInstance()->GetRenderTaskRunner()->WaitForSync();
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
}

void Graphics::PresentScreenInternal() {
  CompositeScreenInternal();

  gpu::Blt::BeginScreen(resolution_);
  gpu::Blt::TexSource(screen_buffer_[0]);

  gpu::GSM.states.clear_color.Set(base::Vec4());
  gpu::FrameBuffer::Clear();

  base::Rect target_rect(0, resolution_.y, resolution_.x, -resolution_.y);
  gpu::Blt::EndDraw(resolution_, target_rect);

  SDL_GL_SwapWindow(window_->AsSDLWindow());
}

}  // namespace content
