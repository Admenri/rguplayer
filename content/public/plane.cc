// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/plane.h"

#include <algorithm>

namespace {

float fwrap(float value, float range) {
  float res = std::fmod(value, range);
  return res < 0 ? res + range : res;
}

}  // namespace

namespace content {

Plane::Plane(scoped_refptr<Graphics> screen, scoped_refptr<Viewport> viewport)
    : GraphicElement(screen),
      Disposable(screen.get()),
      ViewportChild(screen, viewport),
      color_(new Color()),
      tone_(new Tone()) {
  quad_array_ = new renderer::QuadDrawableArray<renderer::CommonVertex>(false);
  screen->renderer()->PostTask(base::BindOnce(
      [](renderer::QuadDrawableArray<renderer::CommonVertex>* quad_ptr) {
        quad_ptr->Init();
        quad_ptr->Resize(1);
      },
      quad_array_));

  layer_tfb_ = screen->AllocTexture(base::Vec2i(16, 16), true);
  OnParentViewportRectChanged(parent_rect());
}

Plane::~Plane() {
  Dispose();
}

void Plane::SetBitmap(scoped_refptr<Bitmap> bitmap) {
  CheckIsDisposed();

  if (bitmap_ == bitmap)
    return;
  bitmap_ = bitmap;

  if (IsObjectValid(bitmap_.get()))
    quad_array_dirty_ = true;
}

void Plane::SetOX(int ox) {
  CheckIsDisposed();

  if (ox_ == ox)
    return;

  ox_ = ox;
  quad_array_dirty_ = true;
}

void Plane::SetOY(int oy) {
  CheckIsDisposed();

  if (oy_ == oy)
    return;

  oy_ = oy;
  quad_array_dirty_ = true;
}

void Plane::SetZoomX(double zoom_x) {
  CheckIsDisposed();

  if (zoom_x_ == zoom_x)
    return;

  zoom_x_ = zoom_x;
  quad_array_dirty_ = true;
}

void Plane::SetZoomY(double zoom_y) {
  CheckIsDisposed();

  if (zoom_y_ == zoom_y)
    return;

  zoom_y_ = zoom_y;
  quad_array_dirty_ = true;
}

void Plane::OnObjectDisposed() {
  RemoveFromList();

  screen()->renderer()->DeleteSoon(std::move(quad_array_));
  screen()->FreeTexture(layer_tfb_);
}

void Plane::BeforeComposite() {
  if (quad_array_dirty_) {
    UpdateQuadArray();
    quad_array_dirty_ = false;
  }
}

void Plane::Composite() {
  if (!opacity_)
    return;
  if (!IsObjectValid(bitmap_.get()))
    return;

  if (color_->IsValid() || tone_->IsValid() || opacity_ != 255) {
    auto& shader = renderer::GSM.shaders()->plane;
    shader.Bind();
    shader.SetProjectionMatrix(renderer::GSM.states.viewport.Current().Size());
    shader.SetTransOffset(parent_rect().GetRealOffset());
    shader.SetTexture(layer_tfb_->tex);
    shader.SetTextureSize(layer_tfb_->size);

    shader.SetColor(color_->AsBase());
    shader.SetTone(tone_->AsBase());
    shader.SetOpacity(opacity_ / 255.0f);
  } else {
    auto& shader = renderer::GSM.shaders()->base;
    shader.Bind();
    shader.SetProjectionMatrix(renderer::GSM.states.viewport.Current().Size());
    shader.SetTransOffset(parent_rect().GetRealOffset());
    shader.SetTexture(layer_tfb_->tex);
    shader.SetTextureSize(layer_tfb_->size);
  }

  renderer::GSM.states.blend.Push(true);
  renderer::GSM.states.blend_func.Push(blend_type_);
  quad_array_->Draw();
  renderer::GSM.states.blend_func.Pop();
  renderer::GSM.states.blend.Pop();
}

void Plane::OnParentViewportRectChanged(const DrawableParent::ViewportInfo&) {
  quad_array_dirty_ = true;
}

void Plane::UpdateQuadArray() {
  if (!IsObjectValid(bitmap_.get()))
    return;

  const float scale_width =
      std::max(1.0f, static_cast<float>(bitmap_->GetWidth() * zoom_x_));
  const float scale_height =
      std::max(1.0f, static_cast<float>(bitmap_->GetHeight() * zoom_y_));

  const int repeat_x =
      static_cast<int>(std::sqrt(parent_rect().rect.width / scale_width)) + 1;
  const int repeat_y =
      static_cast<int>(std::sqrt(parent_rect().rect.height / scale_height)) + 1;

  renderer::TextureFrameBuffer::Alloc(
      *layer_tfb_,
      base::Vec2i(scale_width * repeat_x, scale_height * repeat_y));

  const float item_x = scale_width * repeat_x;
  const float item_y = scale_height * repeat_y;

  auto tex = bitmap_->GetSize();
  quad_array_->Resize(repeat_x * repeat_y);
  for (int y = 0; y < repeat_y; ++y) {
    for (int x = 0; x < repeat_x; ++x) {
      size_t index = (y * repeat_x + x) * 4;
      renderer::CommonVertex* vert = &quad_array_->vertices()[index];
      base::RectF pos(x * scale_width, y * scale_height, scale_width,
                      scale_height);

      renderer::QuadSetPositionRect(vert, pos);
      renderer::QuadSetTexCoordRect(vert, base::Rect(tex));
    }
  }

  quad_array_->Update();

  renderer::FrameBuffer::Bind(layer_tfb_->fbo);
  renderer::FrameBuffer::Clear();

  auto element_size =
      base::Vec2i(static_cast<int>(item_x), static_cast<int>(item_y));
  auto& shader = renderer::GSM.shaders()->base;
  shader.Bind();
  shader.SetProjectionMatrix(element_size);
  shader.SetTexture(bitmap_->GetRaw()->tex);
  shader.SetTextureSize(tex);
  shader.SetTransOffset(base::Vec2i());

  renderer::GSM.states.viewport.Push(element_size);
  renderer::GSM.states.blend.Push(false);
  quad_array_->Draw();
  renderer::GSM.states.viewport.Pop();
  renderer::GSM.states.blend.Pop();

  float wrap_ox = fwrap(ox_, item_x);
  float wrap_oy = fwrap(oy_, item_y);

  int tile_x =
      std::ceil((parent_rect().rect.width + wrap_ox - item_x) / item_x) + 1;
  int tile_y =
      std::ceil((parent_rect().rect.height + wrap_oy - item_y) / item_y) + 1;

  quad_array_->Resize(tile_x * tile_y);

  for (int y = 0; y < tile_y; ++y) {
    for (int x = 0; x < tile_x; ++x) {
      size_t index = (y * tile_x + x) * 4;
      renderer::CommonVertex* vert = &quad_array_->vertices()[index];
      base::RectF pos(x * item_x - wrap_ox, y * item_y - wrap_oy, item_x,
                      item_y);

      renderer::QuadSetPositionRect(vert, pos);
      renderer::QuadSetTexCoordRect(vert, base::Vec2(item_x, item_y));
    }
  }

  quad_array_->Update();
}

}  // namespace content
