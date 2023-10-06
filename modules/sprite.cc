// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "modules/sprite.h"

namespace modules {

Sprite::Sprite(Graphics* screen) : ViewportDrawable(screen), screen_(screen) {
  screen_->GetRenderer()->GetRenderThreadRunner()->PostTask(base::BindOnce(
      &Sprite::InitSpriteRendererInternal, weak_ptr_factory_.GetWeakPtr()));
}

Sprite::Sprite(Graphics* screen, scoped_refptr<Viewport> viewport)
    : ViewportDrawable(screen, viewport), screen_(screen) {
  screen_->GetRenderer()->GetRenderThreadRunner()->PostTask(base::BindOnce(
      &Sprite::InitSpriteRendererInternal, weak_ptr_factory_.GetWeakPtr()));
}

Sprite::~Sprite() { Dispose(); }

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

  return src_rect_->GetHeight();
}

void Sprite::SetBitmap(scoped_refptr<Bitmap> bitmap) {
  CheckedForDispose();

  if (bitmap == bitmap_) return;

  bitmap_ = bitmap;

  src_rect_->SetBase(bitmap_->GetRect());
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

scoped_refptr<Rect> Sprite::GetSrcRect() const {
  CheckedForDispose();

  return src_rect_;
}

void Sprite::SetX(int x) {
  CheckedForDispose();

  transform_.SetPosition(base::Vec2i(x, GetY()));
}

int Sprite::GetX() const {
  CheckedForDispose();

  return transform_.GetPosition().x;
}

void Sprite::SetY(int y) {
  CheckedForDispose();

  transform_.SetPosition(base::Vec2i(GetX(), y));
}

int Sprite::GetY() const {
  CheckedForDispose();

  return transform_.GetPosition().y;
}

void Sprite::SetOX(int ox) {
  CheckedForDispose();

  transform_.SetOrigin(base::Vec2i(ox, GetOY()));
}

int Sprite::GetOX() const {
  CheckedForDispose();

  return transform_.GetOrigin().x;
}

void Sprite::SetOY(int oy) {
  CheckedForDispose();

  transform_.SetOrigin(base::Vec2i(GetOX(), oy));
}

int Sprite::GetOY() const {
  CheckedForDispose();

  return transform_.GetOrigin().y;
}

void Sprite::SetZoomX(float zoom_x) {
  CheckedForDispose();

  transform_.SetScale(base::Vec2(zoom_x, GetZoomY()));
}

float Sprite::GetZoomX() const {
  CheckedForDispose();

  return transform_.GetScale().x;
}

void Sprite::SetZoomY(float zoom_y) {
  CheckedForDispose();

  transform_.SetScale(base::Vec2(GetZoomX(), zoom_y));
}

float Sprite::GetZoomY() const {
  CheckedForDispose();

  return transform_.GetScale().y;
}

void Sprite::SetAngle(float angle) {
  CheckedForDispose();

  transform_.SetRotation(angle);
}

float Sprite::GetAngle() const {
  CheckedForDispose();

  return transform_.GetRotation();
}

void Sprite::SetWaveAmp(int wave_amp) { CheckedForDispose(); }

int Sprite::GetWaveAmp() const {
  CheckedForDispose();

  return 0;
}

void Sprite::SetWaveLength(int wave_length) { CheckedForDispose(); }

int Sprite::GetWaveLength() const {
  CheckedForDispose();

  return 0;
}

void Sprite::SetWaveSpeed(int wave_speed) { CheckedForDispose(); }

int Sprite::GetWaveSpeed() const {
  CheckedForDispose();

  return 0;
}

void Sprite::SetWavePhase(int wave_phase) { CheckedForDispose(); }

int Sprite::GetWavePhase() const {
  CheckedForDispose();

  return 0;
}

void Sprite::SetMirror(bool mirror) {
  CheckedForDispose();

  mirror_ = mirror;
}

bool Sprite::GetMirror() const {
  CheckedForDispose();

  return mirror_;
}

void Sprite::SetBushDepth(int depth) { CheckedForDispose(); }

int Sprite::GetBushDepth() const {
  CheckedForDispose();

  return 0;
}

void Sprite::SetBushOpacity(int opacity) { CheckedForDispose(); }

int Sprite::GetBushOpacity() const {
  CheckedForDispose();

  return 0;
}

void Sprite::SetOpacity(int opacity) {
  CheckedForDispose();

  opacity_ = opacity;
  fopacity_ = std::clamp(opacity, 0, 255) / 255.0f;
}

int Sprite::GetOpacity() const {
  CheckedForDispose();

  return opacity_;
}

void Sprite::SetBlendMode(renderer::BlendMode mode) {
  CheckedForDispose();

  blend_mode_ = mode;
}

renderer::BlendMode Sprite::GetBlendMode() const {
  CheckedForDispose();

  return blend_mode_;
}

void Sprite::SetColor(scoped_refptr<Color> color) {
  CheckedForDispose();

  color_ = color;
}

scoped_refptr<Color> Sprite::GetColor() const {
  CheckedForDispose();

  return color_;
}

void Sprite::SetTone(scoped_refptr<Tone> tone) {
  CheckedForDispose();

  tone_ = tone;
}

scoped_refptr<Tone> Sprite::GetTone() const {
  CheckedForDispose();

  return tone_;
}

void Sprite::InitRefCountedAttributes() {
  src_rect_ = new Rect();
  color_ = new Color();
  tone_ = new Tone();

  src_rect_observer_ = src_rect_->AddObserver(base::BindRepeating(
      &Sprite::OnSrcRectChanged, weak_ptr_factory_.GetWeakPtr()));
}

void Sprite::InitSpriteRendererInternal() {
  auto* cc = content::RendererThread::GetCCForRenderer();

  quad_.reset(
      new renderer::QuadDrawable(cc->GetQuadIndicesBuffer(), cc->GetContext()));
}

void Sprite::OnSrcRectChangedInternal() {
  base::Rect src_rect = src_rect_->AsBase();
  base::Vec2i texture_size = bitmap_->GetSize();

  src_rect.width = std::clamp(src_rect.width, 0, texture_size.x - src_rect.x);
  src_rect.height = std::clamp(src_rect.height, 0, texture_size.y - src_rect.y);
  quad_->SetTexcoord(src_rect);

  quad_->SetPosition(base::Rect(base::Vec2i(), src_rect.Size()));
}

void Sprite::OnObjectDisposed() {
  screen_->GetRenderer()->GetRenderThreadRunner()->DeleteSoon(std::move(quad_));
}

void Sprite::Paint() {
  auto* cc = content::RendererThread::GetCCForRenderer();

  if (!bitmap_) return;

  auto* shader = cc->Shaders()->drawable_shader.get();

  shader->Bind();
  shader->SetViewportMatrix(cc->States()->viewport->Current().Size());
  shader->SetTransformMatrix(transform_.GetMatrixDataUnsafe());

  shader->SetTexture(bitmap_->GetGLTexture()->GetTextureRaw());
  shader->SetTextureSize(bitmap_->GetSize());

  cc->States()->blend->Push(true);
  cc->States()->blend_mode->Push(blend_mode_);

  quad_->Draw();

  cc->States()->blend_mode->Pop();
  cc->States()->blend->Pop();
}

void Sprite::ViewportRectChanged(
    const DrawableManager::DrawableViewport& viewport) {
  transform_.SetGlobalOffset(viewport.GetRealOffset());
}

void Sprite::OnSrcRectChanged() {
  screen_->GetRenderer()->GetRenderThreadRunner()->PostTask(base::BindOnce(
      &Sprite::OnSrcRectChangedInternal, weak_ptr_factory_.GetWeakPtr()));
}

}  // namespace modules
