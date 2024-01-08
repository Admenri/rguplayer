// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/sprite.h"

#include <math.h>

#define M_PI 3.1415926

namespace content {

Sprite::Sprite(scoped_refptr<Graphics> screen, scoped_refptr<Viewport> viewport)
    : GraphicElement(screen),
      Disposable(screen),
      ViewportChild(screen, viewport) {
  InitAttributeInternal();
}

Sprite::~Sprite() {
  Dispose();
}

void Sprite::SetBitmap(scoped_refptr<Bitmap> bitmap) {
  CheckIsDisposed();

  if (bitmap == bitmap_)
    return;

  bitmap_ = bitmap;

  if (bitmap->IsDisposed())
    return;

  src_rect_ = bitmap->GetRect();
  OnSrcRectChangedInternal();
}

void Sprite::SetSrcRect(scoped_refptr<Rect> rect) {
  CheckIsDisposed();
  if (src_rect_ == rect)
    return;

  src_rect_ = rect;
  OnSrcRectChangedInternal();
}

void Sprite::SetMirror(bool mirror) {
  CheckIsDisposed();

  if (mirror_ == mirror)
    return;

  mirror_ = mirror;
  OnSrcRectChangedInternal();
}

void Sprite::Update() {
  Flashable::Update();

  wave_.phase_ += wave_.speed_ / 180.0f;
  wave_.need_update_ = true;
}

void Sprite::InitAttributeInternal() {
  src_rect_ = new Rect();

  color_ = new Color();
  tone_ = new Tone();

  src_rect_observer_ = src_rect_->AddChangedObserver(base::BindRepeating(
      &Sprite::OnSrcRectChangedInternal, weak_ptr_factory_.GetWeakPtr()));

  screen()->renderer()->PostTask(base::BindOnce(
      &Sprite::InitSpriteInternal, weak_ptr_factory_.GetWeakPtr()));

  if (auto* viewport = GetViewport().get())
    OnViewportRectChanged(parent_rect());
}

void Sprite::InitSpriteInternal() {
  quad_ = std::make_unique<renderer::QuadDrawable>();
  wave_quads_ =
      std::make_unique<renderer::QuadDrawableArray<renderer::CommonVertex>>();
}

void Sprite::OnObjectDisposed() {
  RemoveFromList();

  weak_ptr_factory_.InvalidateWeakPtrs();

  screen()->renderer()->DeleteSoon(std::move(quad_));
  screen()->renderer()->DeleteSoon(std::move(wave_quads_));
}

void Sprite::BeforeComposite() {
  if (wave_.need_update_) {
    UpdateWaveQuadsInternal();
    wave_.need_update_ = false;
  }
}

void Sprite::Composite() {
  if (Flashable::IsFlashEmpty())
    return;

  if (!opacity_)
    return;
  if (!bitmap_ || bitmap_->IsDisposed())
    return;

  auto& shader = renderer::GSM.shaders->sprite;

  shader.Bind();
  shader.SetProjectionMatrix(renderer::GSM.states.viewport.Current().Size());
  shader.SetTransformMatrix(transform_.GetMatrixDataUnsafe());

  auto& bitmap_size = bitmap_->AsGLType();
  shader.SetTexture(bitmap_size.tex);
  shader.SetTextureSize(base::Vec2i(bitmap_size.width, bitmap_size.height));
  shader.SetOpacity(opacity_ / 255.0f);

  const base::Vec4 color = color_->AsBase();
  shader.SetColor(
      (Flashable::IsFlashing() && Flashable::GetFlashColor().w > color.w)
          ? Flashable::GetFlashColor()
          : color);

  shader.SetTone(tone_->AsBase());
  shader.SetBushDepth(src_rect_->GetY() + src_rect_->GetHeight() -
                      bush_.depth_);
  shader.SetBushOpacity(bush_.opacity_ / 255.0f);

  renderer::GSM.states.blend.Push(true);
  renderer::GSM.states.blend_func.Push(blend_mode_);

  if (wave_.active_)
    wave_quads_->Draw();
  else
    quad_->Draw();

  renderer::GSM.states.blend_func.Pop();
  renderer::GSM.states.blend.Pop();
}

void Sprite::OnViewportRectChanged(const DrawableParent::ViewportInfo& rect) {
  screen()->renderer()->PostTask(
      base::BindOnce(&Sprite::OnViewportRectChangedInternal,
                     weak_ptr_factory_.GetWeakPtr(), rect));
}

void Sprite::OnSrcRectChangedInternal() {
  screen()->renderer()->PostTask(base::BindOnce(
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

void Sprite::OnViewportRectChangedInternal(
    const DrawableParent::ViewportInfo& rect) {
  transform_.SetGlobalOffset(rect.GetRealOffset());
}

void Sprite::UpdateWaveQuadsInternal() {
  // Wave from other runtime
  // TODO: [deprecated] enhance wave process
  auto emitWaveChunk = [this](renderer::CommonVertex*& vert, float phase,
                              int width, float zoomY, int chunkY,
                              int chunkLength) {
    float wavePos = phase + (chunkY / (float)wave_.length_) * (float)(M_PI * 2);
    float chunkX = std::sin(wavePos) * wave_.amp_;

    base::Rect tex(src_rect_->GetX(), src_rect_->GetY() + chunkY / zoomY, width,
                   chunkLength / zoomY);
    base::Rect pos = tex;
    pos.x = chunkX;

    renderer::QuadSetTexPosRect(vert, tex, pos);
    vert += 4;
  };

  if (!wave_.amp_) {
    wave_.active_ = false;
    return;
  }

  wave_.active_ = true;

  int width = src_rect_->GetWidth();
  int height = src_rect_->GetHeight();
  float zoomY = transform_.GetScale().y;

  if (wave_.amp_ < -(width / 2)) {
    wave_quads_->Resize(0);
    wave_quads_->Update();

    return;
  }

  /* The length of the sprite as it appears on screen */
  int visibleLength = (int)(height * zoomY);

  /* First chunk length (aligned to 8 pixel boundary */
  int firstLength = ((int)transform_.GetPosition().y) % 8;

  /* Amount of full 8 pixel chunks in the middle */
  int chunks = (visibleLength - firstLength) / 8;

  /* Final chunk length */
  int lastLength = (visibleLength - firstLength) % 8;

  wave_quads_->Resize(!!firstLength + chunks + !!lastLength);
  renderer::CommonVertex* vert = &wave_quads_->vertices()[0];

  float phase = (wave_.phase_ * (float)M_PI) / 180.0f;

  if (firstLength > 0)
    emitWaveChunk(vert, phase, width, zoomY, 0, firstLength);

  for (int i = 0; i < chunks; ++i)
    emitWaveChunk(vert, phase, width, zoomY, firstLength + i * 8, 8);

  if (lastLength > 0)
    emitWaveChunk(vert, phase, width, zoomY, firstLength + chunks * 8,
                  lastLength);

  wave_quads_->Update();
}

}  // namespace content
