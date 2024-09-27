// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RENDERER_DRAWABLE_QUAD_DRAWABLE_H_
#define RENDERER_DRAWABLE_QUAD_DRAWABLE_H_

#include "base/memory/ref_counted.h"
#include "renderer/drawable/input_layout.h"

#include <vector>

namespace renderer {

class QuadArrayIndices : public base::RefCounted<QuadArrayIndices> {
 public:
  QuadArrayIndices();
  ~QuadArrayIndices();

  QuadArrayIndices(const QuadArrayIndices&) = delete;
  QuadArrayIndices& operator=(const QuadArrayIndices&) = delete;

  void EnsureSize(size_t count);
  inline bgfx::IndexBufferHandle GetBufferHandle() const { return handle_; }

 private:
  bgfx::IndexBufferHandle handle_;
  std::vector<uint16_t> buffer_;
};

class QuadDrawable {
 public:
  using VertexInput = GeometryVertexLayout;

  QuadDrawable(scoped_refptr<QuadArrayIndices> indices);
  ~QuadDrawable();

  QuadDrawable(const QuadDrawable&) = delete;
  QuadDrawable& operator=(const QuadDrawable&) = delete;

  void SetPosition(const base::RectF& pos);
  void SetTexcoord(const base::RectF& tex);
  void SetColor(const base::Vec4& color, int index = -1);

  void Draw(bgfx::Encoder* encoder,
            bgfx::ProgramHandle shader,
            bgfx::ViewId view = 0);

 private:
  bgfx::VertexLayout vertex_layout_;
  VertexInput::Data raw_data_[4];
  scoped_refptr<QuadArrayIndices> indices_;
  bgfx::DynamicVertexBufferHandle buffer_handle_;
  bool need_update_;
};

}  // namespace renderer

#endif  //! RENDERER_DRAWABLE_QUAD_DRAWABLE_H_
