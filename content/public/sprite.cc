// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/sprite.h"

#include <math.h>

#if !defined(M_PI)
#define M_PI 3.1415926
#endif

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

  if (bitmap_ && !bitmap_->IsDisposed()) {
    src_rect_->Set(bitmap_->GetSize());
    OnSrcRectChangedInternal();
  }
}

void Sprite::SetSrcRect(scoped_refptr<Rect> rect) {
  CheckIsDisposed();

  if (src_rect_->IsSame(*rect))
    return;

  *src_rect_ = *rect;
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
  CheckIsDisposed();
  Flashable::Update();

  wave_.phase += wave_.speed / 180.0f;
  wave_.need_update = true;
}

void Sprite::InitAttributeInternal() {
  color_ = new Color();
  tone_ = new Tone();

  src_rect_ = new Rect();
  src_rect_observer_ = src_rect_->AddChangedObserver(base::BindRepeating(
      &Sprite::OnSrcRectChangedInternal, base::Unretained(this)));

  OnViewportRectChanged(parent_rect());
}

void Sprite::OnObjectDisposed() {
  RemoveFromList();

  quad_.reset();
  wave_quads_.reset();
}

void Sprite::InitDrawableData() {
  quad_ = std::make_unique<renderer::QuadDrawable>();
  wave_quads_ =
      std::make_unique<renderer::QuadDrawableArray<renderer::CommonVertex>>();
}

void Sprite::BeforeComposite() {
  UpdateVisibilityInternal();
  if (need_invisible_)
    return;

  if (src_rect_need_update_) {
    src_rect_need_update_ = false;

    if (bitmap_ && !bitmap_->IsDisposed()) {
      auto bitmap_size = bitmap_->GetSize();
      auto rect = src_rect_->AsBase();

      rect.width = std::clamp(rect.width, 0, bitmap_size.x - rect.x);
      rect.height = std::clamp(rect.height, 0, bitmap_size.y - rect.y);

      quad_->SetPositionRect(base::Vec2(static_cast<float>(rect.width),
                                        static_cast<float>(rect.height)));
      if (mirror_) {
        quad_->SetTexCoordRect(
            base::Rect(rect.x + rect.width, rect.y, -rect.width, rect.height));
      } else {
        quad_->SetTexCoordRect(rect);
      }
    }
  }

  if (wave_.need_update) {
    UpdateWaveQuadsInternal();
    wave_.need_update = false;
  }
}

void Sprite::Composite() {
  if (need_invisible_)
    return;
  if (Flashable::IsFlashing() && Flashable::EmptyFlashing())
    return;

  const bool color_effect = color_->IsValid();
  const bool tone_effect = tone_->IsValid();
  const bool flash_effect = Flashable::IsFlashing();
  const bool bush_effect = bush_.depth != 0;
  const bool render_effect =
      color_effect || tone_effect || flash_effect || bush_effect;

  auto& bitmap_texture = bitmap_->GetTexture();
  if (render_effect) {
    auto& shader = renderer::GSM.shaders()->sprite;
    shader.Bind();
    shader.SetProjectionMatrix(renderer::GSM.states.viewport.Current().Size());
    shader.SetTransformMatrix(transform_.GetMatrixDataUnsafe());
    shader.SetTextureSize(bitmap_texture.size);
    shader.SetOpacity(opacity_ / 255.0f);

    const base::Vec4 color = color_->AsBase();
    shader.SetColor(
        (Flashable::IsFlashing() && Flashable::GetFlashColor().w > color.w)
            ? Flashable::GetFlashColor()
            : color);
    shader.SetTone(tone_->AsBase());
    shader.SetBushDepth(static_cast<float>(
        src_rect_->GetY() + src_rect_->GetHeight() - bush_.depth));
    shader.SetBushOpacity(bush_.opacity / 255.0f);
  } else if (opacity_ != 255) {
    auto& shader = renderer::GSM.shaders()->alphasprite;
    shader.Bind();
    shader.SetProjectionMatrix(renderer::GSM.states.viewport.Current().Size());
    shader.SetTransformMatrix(transform_.GetMatrixDataUnsafe());
    shader.SetTextureSize(bitmap_texture.size);
    shader.SetOpacity(opacity_ / 255.0f);
  } else {
    auto& shader = renderer::GSM.shaders()->basesprite;
    shader.Bind();
    shader.SetProjectionMatrix(renderer::GSM.states.viewport.Current().Size());
    shader.SetTransformMatrix(transform_.GetMatrixDataUnsafe());
    shader.SetTextureSize(bitmap_texture.size);
  }

  // Bind texture default
  renderer::GL.BindTexture(GL_TEXTURE_2D, bitmap_->GetTexture().tex.gl);

  renderer::GSM.states.blend.Push(true);
  renderer::GSM.states.blend_func.Push(blend_mode_);

  if (wave_.active)
    wave_quads_->Draw();
  else
    quad_->Draw();

  renderer::GSM.states.blend_func.Pop();
  renderer::GSM.states.blend.Pop();
}

void Sprite::OnViewportRectChanged(const DrawableParent::ViewportInfo& rect) {
  transform_.SetGlobalOffset(rect.GetRealOffset());
}

void Sprite::OnSrcRectChangedInternal() {
  src_rect_need_update_ = true;
}

void Sprite::UpdateWaveQuadsInternal() {
  // Wave from other runtime
  // TODO: [deprecated] enhance wave process
  auto emitWaveChunk = [this](renderer::CommonVertex*& vert, float phase,
                              int width, float zoomY, int chunkY,
                              int chunkLength) {
    float wavePos = phase + (chunkY / (float)wave_.length) * (float)(M_PI * 2);
    float chunkX = std::sin(wavePos) * wave_.amp;

    float chunkOffset = chunkY / zoomY;
    base::RectF tex(static_cast<float>(src_rect_->GetX()),
                    static_cast<float>(src_rect_->GetY() + chunkOffset),
                    static_cast<float>(width),
                    static_cast<float>(chunkLength / zoomY));
    base::RectF pos(base::Vec2(chunkX, chunkOffset), tex.Size());

    if (mirror_) {
      tex.x += tex.width;
      tex.width = -tex.width;
    }

    renderer::QuadSetTexPosRect(vert, tex, pos);
    vert += 4;
  };

  if (!wave_.amp) {
    wave_.active = false;
    return;
  }

  wave_.active = true;

  int width = src_rect_->GetWidth();
  int height = src_rect_->GetHeight();
  float zoomY = transform_.GetScale().y;

  if (wave_.amp < -(width / 2)) {
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
  renderer::CommonVertex* vert = wave_quads_->vertices().data();

  float phase = (wave_.phase * (float)M_PI) / 180.0f;

  if (firstLength > 0)
    emitWaveChunk(vert, phase, width, zoomY, 0, firstLength);

  for (int i = 0; i < chunks; ++i)
    emitWaveChunk(vert, phase, width, zoomY, firstLength + i * 8, 8);

  if (lastLength > 0)
    emitWaveChunk(vert, phase, width, zoomY, firstLength + chunks * 8,
                  lastLength);

  wave_quads_->Update();
}

void Sprite::UpdateVisibilityInternal() {
  need_invisible_ = true;

  if (opacity_ == 0)
    return;

  if (!bitmap_ || bitmap_->IsDisposed())
    return;

  if (wave_.active) {
    need_invisible_ = false;
    return;
  }

  const base::Vec2& scale = transform_.GetScale();
  if (scale.x != 1.0f || scale.y != 1.0f || transform_.GetRotation() != 0) {
    need_invisible_ = false;
    return;
  }

  auto& viewport = parent_rect();
  if (!viewport.scissor) {
    need_invisible_ = false;
    return;
  }

  SDL_Rect screen_rect = viewport.rect.ToSDLRect();

  SDL_Rect self_rect;
  self_rect.w = bitmap_->GetWidth();
  self_rect.h = bitmap_->GetHeight();

  auto offset = viewport.GetRealOffset();
  self_rect.x =
      transform_.GetPosition().x - transform_.GetOrigin().x + offset.x;
  self_rect.y =
      transform_.GetPosition().y - transform_.GetOrigin().y + offset.y;

  need_invisible_ = !SDL_HasRectIntersection(&self_rect, &screen_rect);
}

}  // namespace content
