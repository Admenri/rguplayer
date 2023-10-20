// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/script/sprite.h"

#include "content/scheduler/worker_cc.h"

namespace content {

Sprite::Sprite() : ViewportChild(nullptr) { InitAttributeInternal(); }

Sprite::Sprite(scoped_refptr<Viewport> viewport) : ViewportChild(viewport) {
  InitAttributeInternal();

  transform_.SetGlobalOffset(viewport->viewport_rect().GetRealOffset());
}

Sprite::~Sprite() { Dispose(); }

void Sprite::SetBitmap(scoped_refptr<Bitmap> bitmap) {
  CheckIsDisposed();

  if (bitmap == bitmap_) return;

  bitmap_ = bitmap;

  if (bitmap->IsDisposed()) return;

  src_rect_ = bitmap->GetRect();
  OnSrcRectChangedInternal();
}

void Sprite::SetSrcRect(scoped_refptr<Rect> rect) {
  CheckIsDisposed();
  if (src_rect_ == rect) return;

  src_rect_ = rect;
  OnSrcRectChangedInternal();
}

void Sprite::SetMirror(bool mirror) {
  CheckIsDisposed();

  if (mirror_ == mirror) return;

  mirror_ = mirror;
  OnSrcRectChangedInternal();
}

void Sprite::Update() { Flashable::Update(); }

void Sprite::InitAttributeInternal() {
  src_rect_ = new Rect();

  color_ = new Color();
  tone_ = new Tone();

  src_rect_observer_ = src_rect_->AddChangedObserver(base::BindRepeating(
      &Sprite::OnSrcRectChangedInternal, weak_ptr_factory_.GetWeakPtr()));

  BindingRunner::Get()->GetRenderer()->PostTask(base::BindOnce(
      &Sprite::InitSpriteInternal, weak_ptr_factory_.GetWeakPtr()));
}

void Sprite::InitSpriteInternal() {
  quad_ = std::make_unique<gpu::QuadDrawable>();
}

void Sprite::DestroySpriteInternal() { quad_.reset(); }

void Sprite::OnObjectDisposed() {
  BindingRunner::Get()->GetRenderer()->PostTask(base::BindOnce(
      &Sprite::DestroySpriteInternal, weak_ptr_factory_.GetWeakPtr()));
}

void Sprite::Composite() {
  if (Flashable::IsFlashEmpty()) return;

  if (!bitmap_) return;

  auto& shader = gpu::GSM.shaders->transform;

  shader.Bind();
  shader.SetProjectionMatrix(gpu::GSM.states.viewport.Current().Size());
  shader.SetTransformMatrix(transform_.GetMatrixDataUnsafe());

  auto& bitmap_size = bitmap_->AsGLType();
  shader.SetTexture(bitmap_size.tex);
  shader.SetTextureSize(base::Vec2i(bitmap_size.width, bitmap_size.height));

  gpu::GSM.states.blend_func.Push(blend_mode_);
  quad_->Draw();
  gpu::GSM.states.blend_func.Pop();
}

void Sprite::OnViewportRectChanged(const DrawableParent::ViewportInfo& rect) {
  transform_.SetGlobalOffset(rect.GetRealOffset());
}

void Sprite::OnSrcRectChangedInternal() {
  BindingRunner::Get()->GetRenderer()->PostTask(base::BindOnce(
      &Sprite::AsyncSrcRectChangedInternal, weak_ptr_factory_.GetWeakPtr()));
}

void Sprite::AsyncSrcRectChangedInternal() {
  auto bitmap_size = bitmap_->GetSize();
  auto rect = src_rect_->AsBase();

  rect.width = std::clamp(rect.width, 0, bitmap_size.x - rect.x);
  rect.height = std::clamp(rect.height, 0, bitmap_size.y - rect.y);

  quad_->SetPositionRect(base::Vec2(static_cast<float>(rect.width),
                                    static_cast<float>(rect.height)));
  quad_->SetTexCoordRect(rect);
}

}  // namespace content
