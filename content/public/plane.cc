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
    : GraphicsElement(screen.get()),
      Disposable(screen.get()),
      ViewportChild(screen, viewport),
      color_(new Color()),
      tone_(new Tone()) {
  quad_array_ =
      std::make_unique<renderer::QuadArray>(screen->device()->quad_indices());
  quad_array_->Resize(1);

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

  quad_array_.reset();
  bgfx::destroy(cache_layer_.handle);
}

void Plane::PrepareDraw(bgfx::Encoder* encoder, bgfx::ViewId* render_view) {
  if (quad_array_dirty_) {
    UpdateQuadArray(encoder, render_view);
    quad_array_dirty_ = false;
  }
}

void Plane::OnDraw(CompositeTargetInfo* target_info) {
  if (!opacity_)
    return;
  if (!IsObjectValid(bitmap_.get()))
    return;

  auto bitmap_texture = bgfx::getTexture(bitmap_->GetHandle());
  bgfx::ProgramHandle program_handle = BGFX_INVALID_HANDLE;
  if (color_->IsValid() || tone_->IsValid() || opacity_ != 255) {
    auto& shader = screen()->device()->pipelines().plane;

    base::Vec4 offset_texsize = base::MakeVec4(
        parent_rect().GetRealOffset(), base::MakeInvert(cache_layer_.size));
    target_info->encoder->setUniform(shader.OffsetTexSize(), &offset_texsize);
    target_info->encoder->setTexture(0, shader.Texture(), bitmap_texture);

    base::Vec4 ncolor = color_->AsBase(), ntone = tone_->AsBase();
    target_info->encoder->setUniform(shader.Color(), &ncolor);
    target_info->encoder->setUniform(shader.Tone(), &ntone);

    base::Vec4 opacity;
    opacity.x = opacity_ / 255.0f;
    target_info->encoder->setUniform(shader.Opacity(), &opacity);

    program_handle = shader.GetProgram();
  } else {
    auto& shader = screen()->device()->pipelines().base;

    base::Vec4 offset_texsize = base::MakeVec4(
        parent_rect().GetRealOffset(), base::MakeInvert(cache_layer_.size));
    target_info->encoder->setUniform(shader.OffsetTexSize(), &offset_texsize);
    target_info->encoder->setTexture(0, shader.Texture(), bitmap_texture);

    program_handle = shader.GetProgram();
  }

  if (target_info->render_scissor.enable)
    target_info->SetScissorRegion(target_info->render_scissor.region);
  target_info->encoder->setState(renderer::MakeColorBlendState(blend_type_));

  quad_array_->Draw(target_info->encoder, program_handle,
                    target_info->render_view);
}

void Plane::OnParentViewportRectChanged(const DrawableParent::ViewportInfo&) {
  quad_array_dirty_ = true;
}

void Plane::UpdateQuadArray(bgfx::Encoder* encoder, bgfx::ViewId* render_view) {
  if (!IsObjectValid(bitmap_.get()))
    return;

  auto bitmap_size = bitmap_->GetSize();

  const float scale_width =
      std::max(1.0f, static_cast<float>(bitmap_size.x * zoom_x_));
  const float scale_height =
      std::max(1.0f, static_cast<float>(bitmap_size.y * zoom_y_));

  const int repeat_x =
      static_cast<int>(std::sqrt(parent_rect().rect.width / scale_width)) + 1;
  const int repeat_y =
      static_cast<int>(std::sqrt(parent_rect().rect.height / scale_height)) + 1;

  screen()->device()->BindRenderView(*render_view, cache_layer_.size,
                                     cache_layer_.handle, 0);

  if (bgfx::isValid(cache_layer_.handle))
    bgfx::destroy(cache_layer_.handle);
  cache_layer_.size =
      base::Vec2i(scale_width * repeat_x, scale_height * repeat_y);
  cache_layer_.handle =
      bgfx::createFrameBuffer(cache_layer_.size.x, cache_layer_.size.y,
                              bgfx::TextureFormat::RGBA8, BGFX_TEXTURE_RT);

  const float item_x = scale_width * repeat_x;
  const float item_y = scale_height * repeat_y;

  auto tex = bitmap_->GetSize();
  quad_array_->Resize(repeat_x * repeat_y);
  for (int y = 0; y < repeat_y; ++y) {
    for (int x = 0; x < repeat_x; ++x) {
      size_t index = (y * repeat_x + x) * 4;
      renderer::GeometryVertexLayout::Data* vert =
          &quad_array_->vertices()[index];
      base::RectF pos(x * scale_width, y * scale_height, scale_width,
                      scale_height);

      renderer::GeometryVertexLayout::SetPosition(vert, pos);
      renderer::GeometryVertexLayout::SetTexcoord(vert, base::Rect(tex));
    }
  }

  quad_array_->Update();

  auto element_size =
      base::Vec2i(static_cast<int>(item_x), static_cast<int>(item_y));
  screen()->device()->BindRenderView(*render_view, element_size,
                                     cache_layer_.handle, 0);

  auto& shader = screen()->device()->pipelines().base;
  auto bitmap_texture = bgfx::getTexture(bitmap_->GetHandle());
  base::Vec4 offset_texsize =
      base::MakeVec4(base::Vec2(), base::MakeInvert(tex));
  encoder->setUniform(shader.OffsetTexSize(), &offset_texsize);
  encoder->setTexture(0, shader.Texture(), bitmap_texture);

  quad_array_->Draw(encoder, shader.GetProgram(), *render_view);

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
      renderer::GeometryVertexLayout::Data* vert =
          &quad_array_->vertices()[index];
      base::RectF pos(x * item_x - wrap_ox, y * item_y - wrap_oy, item_x,
                      item_y);

      renderer::GeometryVertexLayout::SetPosition(vert, pos);
      renderer::GeometryVertexLayout::SetTexcoord(vert,
                                                  base::Vec2(item_x, item_y));
    }
  }

  quad_array_->Update();

  // Next render pass
  (*render_view)++;
}

}  // namespace content
