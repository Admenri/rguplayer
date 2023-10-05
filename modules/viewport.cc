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
  viewport_.rect_ = base::Rect(0, 0, screen->GetWidth(), screen->GetHeight());
}

Viewport::Viewport(Graphics* screen, int x, int y, int width, int height)
    : Drawable(screen->GetScreen()),
      screen_(screen),
      color_(nullptr),
      tone_(nullptr) {
  viewport_.rect_ = base::Rect(x, y, width, height);
}

Viewport::Viewport(Graphics* screen, const base::Rect& rect)
    : Drawable(screen->GetScreen()),
      screen_(screen),
      color_(nullptr),
      tone_(nullptr) {
  viewport_.rect_ = rect;
}

Viewport::~Viewport() { Dispose(); }

void Viewport::SetRect(const base::Rect& rect) {
  viewport_.rect_ = rect;

  OnRectChanged();
}

base::Rect Viewport::GetRect() const { return viewport_.rect_; }

void Viewport::SetOX(int ox) {
  CheckedForDispose();

  if (viewport_.original_point_.x == ox) return;

  viewport_.original_point_.x = ox;

  NotifyViewportChanged();
}

int Viewport::GetOX() const { return viewport_.original_point_.x; }

void Viewport::SetOY(int oy) {
  CheckedForDispose();

  if (viewport_.original_point_.y == oy) return;

  viewport_.original_point_.y = oy;

  NotifyViewportChanged();
}

int Viewport::GetOY() const { return viewport_.original_point_.y; }

void Viewport::SetColor(scoped_refptr<Color> color) { color_ = color; }

scoped_refptr<Color> Viewport::GetColor() const { return color_; }

void Viewport::SetTone(scoped_refptr<Tone> tone) { tone_ = tone; }

scoped_refptr<Tone> Viewport::GetTone() const { return tone_; }

void Viewport::InitRefCountedAttributes() {
  rect_ = new Rect(viewport_.rect_);
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
  cc->States()->scissor_region->Push(viewport_.rect_);

  DrawFrame::DrawablesPaint();

  cc->States()->scissor_region->Pop();
  cc->States()->scissor_test->Pop();
}

void Viewport::ViewportChanged(const ViewportInfo& viewport) {}

void Viewport::OnRectChanged() {
  viewport_.rect_ = rect_->AsBase();

  NotifyViewportChanged();
}

}  // namespace modules
