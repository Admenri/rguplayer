// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "modules/sprite.h"

namespace modules {

Sprite::Sprite(Graphics* screen)
    : Drawable(screen->GetScreen()), screen_(screen) {}

Sprite::Sprite(Graphics* screen, Viewport* viewport)
    : Drawable(viewport), screen_(screen) {}

Sprite::~Sprite() {}

void Sprite::Update() {
  CheckedForDispose();

  Flashable::Update();
}

int Sprite::GetWidth() const {
  CheckedForDispose();

  return src_rect_->GetWidth();
}

int Sprite::GetHeight() const {
  CheckedForDispose();

  return src_rect_->GeiHeight();
}

void Sprite::SetBitmap(scoped_refptr<Bitmap> bitmap) {
  CheckedForDispose();

  if (bitmap == bitmap_) return;

  bitmap_ = bitmap;

  *src_rect_ = bitmap_->GetRect();
  OnSrcRectChanged();
}

scoped_refptr<Bitmap> Sprite::GetBitmap() const {
  CheckedForDispose();

  return bitmap_;
}

void Sprite::SetSrcRect(scoped_refptr<Rect> src_rect) {
  CheckedForDispose();

  if (src_rect_ == src_rect) return;

  src_rect_ = src_rect;
  OnSrcRectChanged();
}

scoped_refptr<Rect> Sprite::GetSrcRect() const { return src_rect_; }

void Sprite::SetViewport(scoped_refptr<Viewport> viewport) {}

scoped_refptr<Viewport> Sprite::GetViewport() const { return nullptr; }

void Sprite::SetVisible(bool visible) {}

bool Sprite::GetVisible() const { return false; }

void Sprite::SetX(int x) {}

int Sprite::GetX() const { return 0; }

void Sprite::SetY(int y) {}

int Sprite::GetY() const { return 0; }

void Sprite::SetOX(int ox) {}

int Sprite::GetOX() const { return 0; }

void Sprite::SetOY(int oy) {}

int Sprite::GetOY() const { return 0; }

void Sprite::SetZoomX(double zoom_x) {}

double Sprite::GetZoomX() const { return 0.0; }

void Sprite::SetZoomY(double zoom_y) {}

double Sprite::GetZoomY() const { return 0.0; }

void Sprite::SetAngle(double angle) {}

double Sprite::GetAngle() const { return 0.0; }

void Sprite::SetWaveAmp(int wave_amp) {}

int Sprite::GetWaveAmp() const { return 0; }

void Sprite::SetWaveLength(int wave_length) {}

int Sprite::GetWaveLength() const { return 0; }

void Sprite::SetWaveSpeed(int wave_speed) {}

int Sprite::GetWaveSpeed() const { return 0; }

void Sprite::SetWavePhase(int wave_phase) {}

int Sprite::GetWavePhase() const { return 0; }

void Sprite::SetMirror(bool mirror) {}

bool Sprite::GetMirror() const { return false; }

void Sprite::SetBushDepth(int depth) {}

int Sprite::GetBushDepth() const { return 0; }

void Sprite::SetBushOpacity(int opacity) {}

int Sprite::GetBushOpacity() const { return 0; }

void Sprite::SetOpacity() {}

int Sprite::GetOpacity() const { return 0; }

void Sprite::SetBlendMode(renderer::BlendMode mode) {}

renderer::BlendMode Sprite::GetBlendMode() const {
  return renderer::BlendMode();
}

void Sprite::SetColor(scoped_refptr<Color> color) {}

scoped_refptr<Color> Sprite::GetColor() const { return nullptr; }

void Sprite::SetTone(scoped_refptr<Tone> tone) {}

scoped_refptr<Tone> Sprite::GetTone() const { return nullptr; }

void Sprite::InitRefCountedAttributes() {
  src_rect_ = new Rect();
  color_ = new Color();
  tone_ = new Tone();

  src_rect_observer_ = src_rect_->AddObserver(base::BindRepeating(
      &Sprite::OnSrcRectChanged, weak_ptr_factory_.GetWeakPtr()));
}

void Sprite::OnObjectDisposed() {}

void Sprite::Paint() {}

void Sprite::ViewportChanged(const DrawFrame::ViewportInfo& viewport) {}

void Sprite::OnSrcRectChanged() {}

}  // namespace modules
