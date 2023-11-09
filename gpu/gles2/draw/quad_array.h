// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_GLES2_DRAW_QUAD_ARRAY_H_
#define GPU_GLES2_DRAW_QUAD_ARRAY_H_

#include "gpu/gles2/draw/quad_drawable.h"

namespace gpu {

template <class VertexType>
class QuadDrawableArray final {
 public:
  QuadDrawableArray() : quad_count_(0), vbo_size_(-1) {
    vao_.vbo = VertexBuffer::Gen();
    vao_.ibo = GSM.quad_ibo->ibo;

    VertexArray<VertexType>::Init(vao_);
  }

  ~QuadDrawableArray() {
    VertexArray<VertexType>::Uninit(vao_);

    VertexBuffer::Del(vao_.vbo);
  }

  void Resize(size_t size) {
    vertices_.resize(size * 4);
    quad_count_ = size;
  }

  void Clear() {
    vertices_.clear();
    quad_count_ = 0;
  }

  void Update() {
    VertexBuffer::Bind(vao_.vbo);

    GLsizeiptr size = vertices_.size() * sizeof(VertexType);

    if (size > vbo_size_) {
      VertexBuffer::BufferData(size, &vertices_[0], GL_DYNAMIC_DRAW);
      vbo_size_ = size;

      GSM.quad_ibo->EnsureSize(quad_count_);
    } else {
      VertexBuffer::BufferSubData(0, size, &vertices_[0]);
    }

    VertexBuffer::Unbind();
  }

  void Draw(size_t offset, size_t count) {
    VertexArray<VertexType>::Bind(vao_);

    const char* _offset = (const char*)0 + offset * 6 * sizeof(uint16_t);
    GL.DrawElements(GL_TRIANGLES, (GLsizei)(count * 6), GL_UNSIGNED_SHORT,
                    _offset);

    VertexArray<VertexType>::Unbind();
  }

  void Draw() { Draw(0, quad_count_); }

  std::vector<VertexType>& vertices() { return vertices_; }
  size_t count() const { return quad_count_; }

 private:
  std::vector<VertexType> vertices_;

  VertexArray<VertexType> vao_;

  size_t quad_count_;
  GLsizeiptr vbo_size_;
};

}  // namespace gpu

#endif  // GPU_GLES2_DRAW_QUAD_ARRAY_H_