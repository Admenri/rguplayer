// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "viewport.h"

namespace modules {

Viewport::Viewport(Graphics* screen)
    : Drawable(screen->GetScreen()),
      screen_(screen),
      color_(nullptr),
      tone_(nullptr) {
  drawable_viewport_.rect_ =
      base::Rect(0, 0, screen->GetWidth(), screen->GetHeight());
}

Viewport::Viewport(Graphics* screen, int x, int y, int width, int height)
    : Drawable(screen->GetScreen()),
      screen_(screen),
      color_(nullptr),
      tone_(nullptr) {
  drawable_viewport_.rect_ = base::Rect(x, y, width, height);
}

Viewport::Viewport(Graphics* screen, const base::Rect& rect)
    : Drawable(screen->GetScreen()),
      screen_(screen),
      color_(nullptr),
      tone_(nullptr) {
  drawable_viewport_.rect_ = rect;
}

Viewport::~Viewport() { Dispose(); }

void Viewport::SetRect(scoped_refptr<Rect> rect) {
  rect_ = rect;

  drawable_viewport_.rect_ = rect->AsBase();
  OnRectChanged();
}

scoped_refptr<Rect> Viewport::GetRect() const { return rect_; }

void Viewport::SetOX(int ox) {
  CheckedForDispose();

  if (drawable_viewport_.origin_.x == ox) return;

  drawable_viewport_.origin_.x = ox;

  NotifyViewportChanged();
}

int Viewport::GetOX() const { return drawable_viewport_.origin_.x; }

void Viewport::SetOY(int oy) {
  CheckedForDispose();

  if (drawable_viewport_.origin_.y == oy) return;

  drawable_viewport_.origin_.y = oy;

  NotifyViewportChanged();
}

int Viewport::GetOY() const { return drawable_viewport_.origin_.y; }

void Viewport::SetColor(scoped_refptr<Color> color) { color_ = color; }

scoped_refptr<Color> Viewport::GetColor() const { return color_; }

void Viewport::SetTone(scoped_refptr<Tone> tone) { tone_ = tone; }

scoped_refptr<Tone> Viewport::GetTone() const { return tone_; }

void Viewport::InitRefCountedAttributes() {
  rect_ = new Rect(drawable_viewport_.rect_);
  color_ = new Color();
  tone_ = new Tone();

  rect_observer_ = rect_->AddObserver(base::BindRepeating(
      &Viewport::OnRectChanged, weak_ptr_factory_.GetWeakPtr()));
}

void Viewport::OnObjectDisposed() { UnlinkNode(); }

void Viewport::Paint() {
  if (flash_empty_) return;
  if (drawable_children_.Empty()) return;

  auto* cc = content::RendererThread::GetCCForRenderer();

  cc->States()->scissor_test->Push(true);
  cc->States()->scissor_region->Push(drawable_viewport_.rect_);

  DrawableManager::Composite();

  cc->States()->scissor_region->Pop();
  cc->States()->scissor_test->Pop();
}

void Viewport::ViewportRectChanged(const DrawableViewport& viewport) {}

void Viewport::OnRectChanged() {
  drawable_viewport_.rect_ = rect_->AsBase();

  NotifyViewportChanged();
}

ViewportDrawable::ViewportDrawable(Graphics* screen,
                                   scoped_refptr<Viewport> viewport, int z)
    : Drawable((viewport ? static_cast<DrawableManager*>(viewport.get())
                         : screen->GetScreen()),
               z),
      screen_(screen),
      viewport_(viewport) {}

void ViewportDrawable::SetViewport(scoped_refptr<Viewport> viewport) {
  NeedCheckAccess();

  viewport_ = viewport;

  SetDrawableManager(viewport.get());

  OnViewportChange();
}

scoped_refptr<Viewport> ViewportDrawable::GetViewport() const {
  NeedCheckAccess();

  return viewport_;
}

}  // namespace modules
