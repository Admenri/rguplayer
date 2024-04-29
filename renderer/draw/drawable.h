// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef RENDERER_DRAW_DRAWABLE_H_
#define RENDERER_DRAW_DRAWABLE_H_

#include "renderer/vertex/vertex_set.h"

namespace renderer {

template <typename VertexType, int VertexCount>
class Drawable {
 public:
  Drawable(const GLID<IndexBuffer>& ibo);
  virtual ~Drawable();

  Drawable(const Drawable&) = delete;
  Drawable& operator=(const Drawable&) = delete;

 protected:
  virtual void UpdateBuffer();
  void NotifyUpdate() { need_update_ = true; }

  VertexType* vertices() { return vertex_; }
  VertexArray<VertexType>& vertex_array() { return vertex_array_; }

 private:
  VertexType vertex_[VertexCount];
  VertexArray<VertexType> vertex_array_;
  bool need_update_ = true;
};

template <typename VertexType, int VertexCount>
inline Drawable<VertexType, VertexCount>::Drawable(
    const GLID<IndexBuffer>& ibo) {
  vertex_array_.vbo = VertexBuffer::Gen();
  vertex_array_.ibo = ibo;
  VertexArray<VertexType>::Init(vertex_array_);
}

template <typename VertexType, int VertexCount>
inline Drawable<VertexType, VertexCount>::~Drawable() {
  VertexArray<CommonVertex>::Uninit(vertex_array_);
  VertexBuffer::Del(vertex_array_.vbo);
}

template <typename VertexType, int VertexCount>
inline void Drawable<VertexType, VertexCount>::UpdateBuffer() {
  if (!need_update_)
    return;

  // Upload data
  VertexBuffer::Bind(vertex_array_.vbo);
  VertexBuffer::BufferData(sizeof(vertex_), &vertex_, GL_DYNAMIC_DRAW);
  VertexBuffer::Unbind();

  // Upload complete
  need_update_ = false;
}

}  // namespace renderer

#endif  // !RENDERER_DRAW_DRAWABLE_H_
