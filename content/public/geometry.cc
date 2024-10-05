// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/geometry.h"

namespace content {

Geometry::Geometry(scoped_refptr<Graphics> screen,
                   scoped_refptr<Viewport> viewport)
    : GraphicsElement(screen.get()),
      Disposable(screen.get()),
      ViewportChild(screen, viewport),
      triangle_count_(64) {
  triangle_vertices_.resize(triangle_count_ * 3);
  buffer_need_update_ = true;

  vertex_buffer_ = bgfx::createDynamicVertexBuffer(
      bgfx::copy(triangle_vertices_.data(),
                 triangle_vertices_.size() *
                     sizeof(renderer::GeometryVertexLayout::Data)),
      renderer::GeometryVertexLayout::GetLayout(), BGFX_BUFFER_ALLOW_RESIZE);
}

Geometry::~Geometry() {
  Dispose();
}

void Geometry::SetBitmap(scoped_refptr<Bitmap> bitmap) {
  if (bitmap_ == bitmap)
    return;
  bitmap_ = bitmap;
}

void Geometry::Resize(size_t count) {
  CheckIsDisposed();
  triangle_vertices_.resize(count * 3);
  triangle_count_ = count;
  buffer_need_update_ = true;
}

size_t Geometry::GetCapacity() {
  CheckIsDisposed();
  return triangle_count_;
}

void Geometry::SetPosition(size_t index, const base::Vec4 position) {
  CheckIsDisposed();

  SetPositionInternal(index, position);
}

void Geometry::SetTexcoord(size_t index, const base::Vec2 texcoord) {
  CheckIsDisposed();

  SetTexcoordInternal(index, texcoord);
}

void Geometry::SetColor(size_t index, const base::Vec4 color) {
  CheckIsDisposed();

  SetColorInternal(index, color);
}

void Geometry::OnObjectDisposed() {
  RemoveFromList();

  triangle_vertices_.clear();
  triangle_count_ = 0;

  bgfx::destroy(vertex_buffer_);
}

void Geometry::PrepareDraw(bgfx::Encoder* encoder, bgfx::ViewId* render_view) {
  if (buffer_need_update_) {
    buffer_need_update_ = false;

    bgfx::update(vertex_buffer_, 0,
                 bgfx::copy(triangle_vertices_.data(),
                            triangle_vertices_.size() *
                                sizeof(renderer::GeometryVertexLayout::Data)));
  }
}

void Geometry::OnDraw(CompositeTargetInfo* target_info) {
  const bool texture_valid = IsObjectValid(bitmap_.get());

  auto& shader = screen()->device()->pipelines().geometry;

  base::Vec4 offset_size = base::MakeVec4(
      parent_rect().GetRealOffset(),
      texture_valid ? base::MakeInvert(bitmap_->GetSize()) : base::Vec2());
  target_info->encoder->setUniform(shader.OffsetTexSize(), &offset_size);

  target_info->encoder->setTexture(0, shader.Texture(),
                                   bgfx::getTexture(bitmap_->GetHandle()));

  if (target_info->render_scissor.enable)
    target_info->SetScissorRegion(target_info->render_scissor.region);

  target_info->encoder->setState(renderer::MakeColorBlendState(blend_mode_));
  target_info->encoder->setVertexBuffer(0, vertex_buffer_);
  target_info->encoder->submit(target_info->render_view, shader.GetProgram());
}

void Geometry::SetPositionInternal(size_t index, const base::Vec4 position) {
  size_t size = triangle_vertices_.size();
  index = std::clamp<size_t>(index, 0, size - 1);

  renderer::GeometryVertexLayout::Data* vert = triangle_vertices_.data();
  vert[index].position = position;

  buffer_need_update_ = true;
}

void Geometry::SetTexcoordInternal(size_t index, const base::Vec2 texcoord) {
  size_t size = triangle_vertices_.size();
  index = std::clamp<size_t>(index, 0, size - 1);

  renderer::GeometryVertexLayout::Data* vert = triangle_vertices_.data();
  vert[index].texcoord = texcoord;

  buffer_need_update_ = true;
}

void Geometry::SetColorInternal(size_t index, const base::Vec4 color) {
  size_t size = triangle_vertices_.size();
  index = std::clamp<size_t>(index, 0, size - 1);

  renderer::GeometryVertexLayout::Data* vert = triangle_vertices_.data();
  vert[index].color = color;

  buffer_need_update_ = true;
}

}  // namespace content
