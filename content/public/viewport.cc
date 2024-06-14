// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/viewport.h"

#include "content/public/bitmap.h"
#include "content/public/utility.h"
#include "renderer/quad/quad_drawable.h"

namespace content {

Viewport::Viewport(scoped_refptr<Graphics> screen)
    : GraphicElement(screen),
      Disposable(screen),
      Drawable(screen.get(), 0, true) {
  parent_offset_ = screen->viewport_rect().GetRealOffset();
  viewport_rect() = screen->viewport_rect();
  InitViewportInternal(screen->GetSize());
}

Viewport::Viewport(scoped_refptr<Graphics> screen, const base::Rect& rect)
    : GraphicElement(screen),
      Disposable(screen),
      Drawable(screen.get(), 0, true) {
  parent_offset_ = screen->viewport_rect().GetRealOffset();
  viewport_rect().rect = rect;
  viewport_rect().rect.x += parent_offset_.x;
  viewport_rect().rect.y += parent_offset_.y;
  InitViewportInternal(rect);
}

Viewport::Viewport(scoped_refptr<Graphics> screen,
                   scoped_refptr<Viewport> viewport)
    : GraphicElement(screen),
      Disposable(screen),
      Drawable(viewport.get(), 0, true) {
  auto parent_rect = viewport->viewport_rect();
  parent_offset_ = parent_rect.GetRealOffset();
  viewport_rect().rect = base::Rect(parent_offset_, parent_rect.rect.Size());
  InitViewportInternal(parent_rect.rect.Size());
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

  if (rect->IsSame(*rect_))
    return;

  *rect_ = *rect;
  OnRectChangedInternal();
}

void Viewport::SnapToBitmap(scoped_refptr<Bitmap> target) {
  CheckIsDisposed();

  screen()->renderer()->PostTask(base::BindOnce(&Viewport::SnapToBitmapInternal,
                                                base::Unretained(this),
                                                target->GetRaw()));
  screen()->renderer()->WaitForSync();
}

void Viewport::SetViewport(scoped_refptr<Viewport> viewport) {
  CheckIsDisposed();

  if (viewport == parent_)
    return;
  parent_ = viewport;

  DrawableParent* parent = parent_.get();
  if (!parent)
    parent = screen().get();
  Drawable::SetParent(parent);
  parent_offset_ = parent->viewport_rect().GetRealOffset();
  OnRectChangedInternal();
}

void Viewport::SetShader(scoped_refptr<Shader> shader) {
  CheckIsDisposed();

  if (shader_program_ == shader)
    return;
  shader_program_ = shader;
}

void Viewport::OnObjectDisposed() {
  RemoveFromList();

  screen()->renderer()->DeleteSoon(std::move(viewport_quad_));
  screen()->FreeTexture(viewport_buffer_);
}

void Viewport::BeforeComposite() {
  DrawableParent::NotifyPrepareComposite();
}

void Viewport::Composite() {
  if (!shader_program_ && Flashable::IsFlashing() && Flashable::EmptyFlashing())
    return;

  renderer::GSM.states.scissor.Push(true);
  renderer::GSM.states.scissor_rect.PushOnly();
  renderer::GSM.states.scissor_rect.SetIntersect(viewport_rect().rect);

  DrawableParent::CompositeChildren();
  if (Flashable::IsFlashing() || color_->IsValid() || tone_->IsValid() ||
      (shader_program_ && !shader_program_->IsDisposed())) {
    base::Vec4 composite_color = color_->AsBase();
    base::Vec4 flash_color = Flashable::GetFlashColor();
    base::Vec4 target_color;
    if (Flashable::IsFlashing())
      target_color =
          (flash_color.w > composite_color.w ? flash_color : composite_color);
    else
      target_color = composite_color;

    screen()->RenderEffectRequire(target_color, tone_->AsBase(),
                                  shader_program_);
  }

  renderer::GSM.states.scissor_rect.Pop();
  renderer::GSM.states.scissor.Pop();
}

void Viewport::OnParentViewportRectChanged(const ViewportInfo& rect) {
  parent_offset_ = rect.GetRealOffset();
  OnRectChangedInternal();
}

void Viewport::InitViewportInternal(const base::Rect& initial_rect) {
  rect_ = new Rect(initial_rect);
  rect_observer_ = rect_->AddChangedObserver(base::BindRepeating(
      &Viewport::OnRectChangedInternal, base::Unretained(this)));

  color_ = new Color();
  tone_ = new Tone();

  viewport_buffer_ = screen()->AllocTexture(viewport_rect().rect.Size(), false);

  viewport_quad_ = new renderer::QuadDrawable(false);
  screen()->renderer()->PostTask(base::BindOnce(
      [](renderer::QuadDrawable* quad_ptr, const base::Rect& rect) {
        quad_ptr->SetPositionRect(rect);
        quad_ptr->SetTexCoordRect(rect);
      },
      viewport_quad_, base::Rect(viewport_rect().rect.Size())));
}

void Viewport::OnRectChangedInternal() {
  viewport_rect().rect = rect_->AsBase();
  viewport_rect().rect.x += parent_offset_.x;
  viewport_rect().rect.y += parent_offset_.y;

  NotifyViewportChanged();
  viewport_rect_need_update_ = true;
}

void Viewport::SnapToBitmapInternal(renderer::TextureFrameBuffer* target) {
  if (viewport_rect_need_update_) {
    viewport_rect_need_update_ = false;
    auto rect = base::Rect(viewport_rect().rect.Size());
    viewport_quad_->SetPositionRect(rect);
    viewport_quad_->SetTexCoordRect(rect);

    renderer::TextureFrameBuffer::Alloc(*viewport_buffer_,
                                        viewport_rect().rect.Size());
  }

  NotifyPrepareComposite();

  renderer::FrameBuffer::Bind(target->fbo);
  renderer::GSM.states.clear_color.Set(base::Vec4());
  renderer::FrameBuffer::Clear();

  if (Flashable::IsFlashing() && Flashable::EmptyFlashing())
    return;

  renderer::GSM.states.scissor.Push(true);
  renderer::GSM.states.scissor_rect.Push(viewport_rect().rect);

  CompositeChildren();
  if (Flashable::IsFlashing() || color_->IsValid() || tone_->IsValid() ||
      (shader_program_ && !shader_program_->IsDisposed())) {
    base::Vec4 composite_color = color_->AsBase();
    base::Vec4 flash_color = Flashable::GetFlashColor();
    base::Vec4 target_color;
    if (Flashable::IsFlashing())
      target_color =
          (flash_color.w > composite_color.w ? flash_color : composite_color);
    else
      target_color = composite_color;

    screen()->ApplyViewportEffect(*target, *viewport_buffer_, *viewport_quad_,
                                  target_color, tone_->AsBase(),
                                  shader_program_);
  }

  renderer::GSM.states.scissor_rect.Pop();
  renderer::GSM.states.scissor.Pop();
}

ViewportChild::ViewportChild(scoped_refptr<Graphics> screen,
                             scoped_refptr<Viewport> viewport,
                             int z,
                             int sprite_y)
    : Drawable(viewport ? static_cast<DrawableParent*>(viewport.get())
                        : screen.get(),
               z,
               true,
               sprite_y),
      screen_(screen) {}

void ViewportChild::SetViewport(scoped_refptr<Viewport> viewport) {
  CheckDisposed();

  if (viewport == viewport_)
    return;
  viewport_ = viewport;

  DrawableParent* parent = viewport_.get();
  if (!parent)
    parent = screen_.get();
  SetParent(parent);
  OnParentViewportRectChanged(parent->viewport_rect());
}

}  // namespace content