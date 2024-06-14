// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef RENDERER_QUAD_QUAD_ARRAY_H_
#define RENDERER_QUAD_QUAD_ARRAY_H_

#include "renderer/quad/quad_drawable.h"
#include "renderer/thread/thread_manager.h"

namespace renderer {

template <class VertexType>
class QuadDrawableArray final {
 public:
  QuadDrawableArray(bool init = true) {
    if (init)
      Init();
  }

  ~QuadDrawableArray() {
    VertexArray<VertexType>::Uninit(vao_);

    VertexBuffer::Del(vao_.vbo);
  }

  inline void Init() {
    vao_.vbo = VertexBuffer::Gen();
    vao_.ibo = GSM.quad_ibo()->ibo;

    VertexArray<VertexType>::Init(vao_);
  }

  inline void Resize(size_t size) {
    vertices_.resize(size * 4);
    quad_size_ = size;
  }

  inline void Clear() {
    vertices_.clear();
    quad_size_ = 0;
  }

  inline void Update() {
    if (!quad_size_)
      return;

    VertexBuffer::Bind(vao_.vbo);
    size_t buffer_size = quad_size_ * sizeof(VertexType) * 4;
    if (buffer_size > vbo_size_) {
      VertexBuffer::BufferData(buffer_size, vertices_.data(), GL_DYNAMIC_DRAW);
      vbo_size_ = buffer_size;

      // Ensure ibo max index
      GSM.quad_ibo()->EnsureSize(quad_size_);
    } else {
      // As subdata upload
      VertexBuffer::BufferSubData(0, buffer_size, vertices_.data());
    }
    VertexBuffer::Unbind();
  }

  inline void Draw(size_t offset, size_t count) {
    if (!count)
      return;

    VertexArray<VertexType>::Bind(vao_);
    const char* _offset = (const char*)0 + offset * 6 * sizeof(uint16_t);
    GL.DrawElements(GL_TRIANGLES, (GLsizei)(count * 6), GL_UNSIGNED_SHORT,
                    _offset);
    VertexArray<VertexType>::Unbind();
  }

  inline void Draw() { Draw(0, quad_size_); }

  inline std::vector<VertexType>& vertices() { return vertices_; }
  inline size_t count() const { return quad_size_; }

 private:
  std::vector<VertexType> vertices_;
  size_t quad_size_ = 0;
  VertexArray<VertexType> vao_;
  size_t vbo_size_ = 0;
};

}  // namespace renderer

#endif  // !RENDERER_QUAD_QUAD_ARRAY_H_
