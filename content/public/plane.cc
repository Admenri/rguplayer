// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/plane.h"

namespace {

float fwrap(float value, float range) {
  float res = std::fmod(value, range);
  return res < 0 ? res + range : res;
}

}  // namespace

namespace content {

Plane::Plane(scoped_refptr<Graphics> screen, scoped_refptr<Viewport> viewport)
    : GraphicElement(screen),
      Disposable(screen),
      ViewportChild(screen, viewport),
      color_(new Color()),
      tone_(new Tone()) {
  screen->renderer()->PostTask(base::BindOnce(&Plane::InitPlaneInternal,
                                              weak_ptr_factory_.GetWeakPtr()));

  if (auto* viewport = GetViewport().get())
    OnViewportRectChanged(parent_rect());
}

Plane::~Plane() {
  Dispose();
}

void Plane::SetBitmap(scoped_refptr<Bitmap> bitmap) {
  CheckIsDisposed();

  if (bitmap_ == bitmap)
    return;

  bitmap_ = bitmap;
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

  weak_ptr_factory_.InvalidateWeakPtrs();

  screen()->renderer()->DeleteSoon(std::move(quad_array_));
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
  if (!bitmap_ || bitmap_->IsDisposed())
    return;

  auto& shader = renderer::GSM.shaders->plane;

  shader.Bind();
  shader.SetProjectionMatrix(renderer::GSM.states.viewport.Current().Size());
  shader.SetTransOffset(parent_rect().GetRealOffset());

  auto& bitmap_size = bitmap_->AsGLType();
  shader.SetTexture(bitmap_size.tex);
  shader.SetTextureSize(base::Vec2i(bitmap_size.width, bitmap_size.height));

  shader.SetColor(color_->AsBase());
  shader.SetTone(tone_->AsBase());
  shader.SetOpacity(opacity_ / 255.0f);

  renderer::GSM.states.blend.Push(true);
  renderer::GSM.states.blend_func.Push(blend_type_);
  quad_array_->Draw();
  renderer::GSM.states.blend_func.Pop();
  renderer::GSM.states.blend.Pop();
}

void Plane::OnViewportRectChanged(const DrawableParent::ViewportInfo& rect) {
  quad_array_dirty_ = true;
}

void Plane::InitPlaneInternal() {
  quad_array_ =
      std::make_unique<renderer::QuadDrawableArray<renderer::CommonVertex>>();

  quad_array_->Resize(1);
}

void Plane::UpdateQuadArray() {
  if (!bitmap_ || bitmap_->IsDisposed())
    return;

  /* Scaled (zoomed) bitmap dimensions */
  float sw = static_cast<float>(bitmap_->GetWidth() * zoom_x_);
  float sh = static_cast<float>(bitmap_->GetHeight() * zoom_y_);

  /* Plane offset wrapped by scaled bitmap dims */
  float wox = fwrap((float)ox_, sw);
  float woy = fwrap((float)oy_, sh);

  /* Viewport dimensions */
  int vpw = parent_rect().rect.width;
  int vph = parent_rect().rect.height;

  /* Amount the scaled bitmap is tiled (repeated) */
  size_t tilesX = (size_t)std::ceil((vpw - sw + wox) / sw) + 1;
  size_t tilesY = (size_t)std::ceil((vph - sh + woy) / sh) + 1;

  auto tex = bitmap_->GetSize();
  quad_array_->Resize(tilesX * tilesY);

  for (size_t y = 0; y < tilesY; ++y) {
    for (size_t x = 0; x < tilesX; ++x) {
      size_t index = (y * tilesX + x) * 4;
      renderer::CommonVertex* vert = &quad_array_->vertices()[index];
      base::RectF pos(x * sw - wox, y * sh - woy, sw, sh);

      renderer::QuadSetPositionRect(vert, pos);
      renderer::QuadSetTexCoordRect(vert, base::Vec2(tex));
    }
  }

  quad_array_->Update();
}

}  // namespace content
