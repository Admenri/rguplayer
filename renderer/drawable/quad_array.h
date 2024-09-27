// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RENDERER_DRAWABLE_QUAD_ARRAY_H_
#define RENDERER_DRAWABLE_QUAD_ARRAY_H_

#include "renderer/drawable/quad_drawable.h"

namespace renderer {

class QuadArray {
 public:
  using Vertex = GeometryVertexLayout;
  using VertexType = GeometryVertexLayout::Data;

  QuadArray(scoped_refptr<QuadArrayIndices> indices);
  ~QuadArray();

  QuadArray(const QuadArray&) = delete;
  QuadArray& operator=(const QuadArray&) = delete;

  void Resize(size_t size);
  void Clear();
  void Update();
  void Draw(bgfx::Encoder* encoder,
            bgfx::ProgramHandle shader,
            size_t offset,
            size_t count,
            bgfx::ViewId view = 0);
  inline void Draw(bgfx::Encoder* encoder,
                   bgfx::ProgramHandle shader,
                   bgfx::ViewId view = 0) {
    Draw(encoder, shader, 0, quad_size_, view);
  }

  inline std::vector<VertexType>& vertices() { return vertices_; }
  inline size_t count() const { return quad_size_; }

 private:
  scoped_refptr<QuadArrayIndices> indices_;
  bgfx::DynamicVertexBufferHandle buffer_handle_;
  std::vector<VertexType> vertices_;
  size_t quad_size_ = 0;
};

}  // namespace renderer

#endif  //! RENDERER_DRAWABLE_QUAD_DRAWABLE_H_
