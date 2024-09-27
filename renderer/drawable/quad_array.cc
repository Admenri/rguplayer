// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "renderer/drawable/quad_array.h"

namespace renderer {

QuadArray::QuadArray(scoped_refptr<QuadArrayIndices> indices)
    : indices_(indices),
      buffer_handle_(bgfx::createDynamicVertexBuffer(4, Vertex::GetLayout())) {}

QuadArray::~QuadArray() {
  if (bgfx::isValid(buffer_handle_))
    bgfx::destroy(buffer_handle_);
}

void QuadArray::Resize(size_t size) {
  vertices_.resize(size * 4);
  quad_size_ = size;
}

void QuadArray::Clear() {
  vertices_.clear();
  quad_size_ = 0;
}

void QuadArray::Update() {
  if (!quad_size_)
    return;

  size_t buffer_size = quad_size_ * sizeof(VertexType) * 4;
  bgfx::update(buffer_handle_, 0, bgfx::copy(vertices_.data(), buffer_size));
  indices_->EnsureSize(quad_size_);
}

void QuadArray::Draw(bgfx::Encoder* encoder,
                     bgfx::ProgramHandle shader,
                     size_t offset,
                     size_t count,
                     bgfx::ViewId view) {
  encoder->setVertexBuffer(0, buffer_handle_);
  encoder->setIndexBuffer(indices_->GetBufferHandle(), offset, count);
  encoder->submit(view, shader);
}

}  // namespace renderer
