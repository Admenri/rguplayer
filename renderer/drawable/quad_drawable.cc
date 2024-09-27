// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "renderer/drawable/quad_drawable.h"

#include <array>

static const std::array<uint16_t, 6> kQuadIndexTemplate = {0, 1, 2, 2, 3, 0};

namespace renderer {

QuadArrayIndices::QuadArrayIndices() : handle_(BGFX_INVALID_HANDLE) {
  EnsureSize(2 << 10);
}

QuadArrayIndices::~QuadArrayIndices() {
  if (bgfx::isValid(handle_))
    bgfx::destroy(handle_);
}

void QuadArrayIndices::EnsureSize(size_t count) {
  if (buffer_.size() >= count * 6)
    return;

  size_t begin = buffer_.size() / 6;
  buffer_.reserve(count * 6);
  for (size_t i = begin; i < count; ++i)
    for (size_t j = 0; j < 6; ++j)
      buffer_.push_back(static_cast<uint16_t>(i * 4 + kQuadIndexTemplate[j]));

  if (bgfx::isValid(handle_))
    bgfx::destroy(handle_);
  handle_ = bgfx::createIndexBuffer(bgfx::copy(buffer_.data(), buffer_.size()),
                                    BGFX_BUFFER_NONE);
}

QuadDrawable::QuadDrawable(scoped_refptr<QuadArrayIndices> indices)
    : vertex_layout_(VertexInput::GetLayout()),
      raw_data_(),
      indices_(indices),
      buffer_handle_(bgfx::createDynamicVertexBuffer(4, vertex_layout_)),
      need_update_(false) {}

QuadDrawable ::~QuadDrawable() {
  if (bgfx::isValid(buffer_handle_))
    bgfx::destroy(buffer_handle_);
}

void QuadDrawable::SetPosition(const base::RectF& pos) {
  VertexInput::SetPosition(raw_data_, pos);
  need_update_ = true;
}

void QuadDrawable::SetTexcoord(const base::RectF& tex) {
  VertexInput::SetTexcoord(raw_data_, tex);
  need_update_ = true;
}

void QuadDrawable::SetColor(const base::Vec4& color, int index) {
  VertexInput::SetColor(raw_data_, color, index);
  need_update_ = true;
}

void QuadDrawable::Draw(bgfx::Encoder* encoder,
                        bgfx::ProgramHandle shader,
                        bgfx::ViewId view) {
  if (need_update_) {
    bgfx::update(buffer_handle_, 0, bgfx::copy(raw_data_, sizeof(raw_data_)));
    need_update_ = false;
  }

  encoder->setVertexBuffer(0, buffer_handle_);
  encoder->setIndexBuffer(indices_->GetBufferHandle(), 0, 6);
  encoder->submit(view, shader);
}

}  // namespace renderer
