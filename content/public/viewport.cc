// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/viewport.h"

#include "content/public/bitmap.h"
#include "renderer/quad/quad_drawable.h"

namespace content {

Viewport::Viewport(scoped_refptr<Graphics> screen)
    : GraphicElement(screen),
      Disposable(screen),
      Drawable(screen.get(), 0, true) {
  viewport_rect().rect = screen->GetSize();
  InitViewportInternal();
}

Viewport::Viewport(scoped_refptr<Graphics> screen, const base::Rect& rect)
    : GraphicElement(screen),
      Disposable(screen),
      Drawable(screen.get(), 0, true) {
  viewport_rect().rect = rect;
  InitViewportInternal();
}

Viewport::~Viewport() {
  Dispose();
}

void Viewport::SetOX(int ox) {
  CheckIsDisposed();

  if (viewport_rect().origin.x == ox)
    return;

  viewport_rect().origin.x = ox;
  NotifyViewportChanged();
}

void Viewport::SetOY(int oy) {
  CheckIsDisposed();

  if (viewport_rect().origin.y == oy)
    return;

  viewport_rect().origin.y = oy;
  NotifyViewportChanged();
}

void Viewport::SetRect(scoped_refptr<Rect> rect) {
  CheckIsDisposed();

  if (*rect == *rect_)
    return;

  *rect_ = *rect;
  OnRectChangedInternal();
}

void Viewport::SnapToBitmap(scoped_refptr<Bitmap> target) {
  CheckIsDisposed();

  screen()->renderer()->PostTask(base::BindOnce(
      &Viewport::SnapToBitmapInternal, weak_ptr_factory_.GetWeakPtr(), target));
}

void Viewport::OnObjectDisposed() {
  RemoveFromList();

  screen()->renderer()->DeleteSoon(std::move(viewport_quad_));
  screen()->renderer()->PostTask(
      base::BindOnce(renderer::TextureFrameBuffer::Del,
                     base::OwnedRef(std::move(viewport_buffer_))));
}

void Viewport::Composite() {
  if (DrawableParent::link().empty() || Flashable::IsFlashEmpty() ||
      !Drawable::GetVisible())
    return;

  renderer::GSM.states.scissor.Push(true);
  renderer::GSM.states.scissor_rect.Push(viewport_rect().rect);

  DrawableParent::CompositeChildren();
  if (Flashable::IsFlashing() || color_->IsValid() || tone_->IsValid()) {
    screen()->RenderEffectRequire(color_->AsBase(), tone_->AsBase(),
                                  Flashable::GetFlashColor());
  }

  renderer::GSM.states.scissor_rect.Pop();
  renderer::GSM.states.scissor.Pop();
}

void Viewport::OnViewportRectChanged(const ViewportInfo& rect) {
  // Bypass, no process.
}

void Viewport::InitViewportInternal() {
  rect_ = new Rect();
  rect_observer_ = rect_->AddChangedObserver(base::BindRepeating(
      &Viewport::OnRectChangedInternal, weak_ptr_factory_.GetWeakPtr()));

  color_ = new Color();
  tone_ = new Tone();

  screen()->renderer()->PostTask(base::BindOnce(
      &Viewport::InitViewportBufferInternal, weak_ptr_factory_.GetWeakPtr()));
}

void Viewport::OnRectChangedInternal() {
  viewport_rect().rect = rect_->AsBase();
  NotifyViewportChanged();

  screen()->renderer()->PostTask(
      base::BindOnce(&Viewport::OnViewportBufferSizeChangedInternal,
                     weak_ptr_factory_.GetWeakPtr()));
}

void Viewport::SnapToBitmapInternal(scoped_refptr<Bitmap> target) {
  NotifyPrepareComposite();

  renderer::FrameBuffer::Bind(target->AsGLType().fbo);
  renderer::GSM.states.clear_color.Set(base::Vec4());
  renderer::FrameBuffer::Clear();

  renderer::GSM.states.scissor.Push(true);
  renderer::GSM.states.scissor_rect.Push(viewport_rect().rect);

  CompositeChildren();
  if (Flashable::IsFlashing() || color_->IsValid() || tone_->IsValid()) {
    screen()->ApplyViewportEffect(target->AsGLType(), viewport_buffer_,
                                  *viewport_quad_, color_->AsBase(),
                                  tone_->AsBase(), Flashable::GetFlashColor());
  }

  renderer::GSM.states.scissor_rect.Pop();
  renderer::GSM.states.scissor.Pop();
}

void Viewport::InitViewportBufferInternal() {
  viewport_buffer_ = renderer::TextureFrameBuffer::Gen();
  renderer::TextureFrameBuffer::Alloc(viewport_buffer_,
                                      viewport_rect().rect.width,
                                      viewport_rect().rect.height);
  renderer::TextureFrameBuffer::LinkFrameBuffer(viewport_buffer_);

  viewport_quad_ = std::make_unique<renderer::QuadDrawable>();
  auto rect = base::Rect(viewport_rect().rect.Size());
  viewport_quad_->SetPositionRect(rect);
  viewport_quad_->SetTexCoordRect(rect);
}

void Viewport::OnViewportBufferSizeChangedInternal() {
  auto rect = base::Rect(viewport_rect().rect.Size());
  viewport_quad_->SetPositionRect(rect);
  viewport_quad_->SetTexCoordRect(rect);

  renderer::TextureFrameBuffer::Alloc(viewport_buffer_,
                                      viewport_rect().rect.width,
                                      viewport_rect().rect.height);
}

ViewportChild::ViewportChild(scoped_refptr<Graphics> screen,
                             scoped_refptr<Viewport> viewport,
                             int z)
    : Drawable(viewport ? static_cast<DrawableParent*>(viewport.get())
                        : screen.get(),
               z,
               true) {}

void ViewportChild::SetViewport(scoped_refptr<Viewport> viewport) {
  CheckDisposed();

  if (viewport == viewport_)
    return;

  viewport_ = viewport;
  SetParent(viewport_.get());
}

}  // namespace content