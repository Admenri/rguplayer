// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef RENDERER_DRAW_DRAWABLE_H_
#define RENDERER_DRAW_DRAWABLE_H_

#include "renderer/vertex/vertex_set.h"

namespace renderer {

template <int VertexCount>
class CommonVertexDrawable {
 public:
  CommonVertexDrawable(const GLID<IndexBuffer>& ibo);
  virtual ~CommonVertexDrawable();

  CommonVertexDrawable(const CommonVertexDrawable&) = delete;
  CommonVertexDrawable& operator=(const CommonVertexDrawable&) = delete;

  virtual void Draw();

 protected:
  virtual void UpdateBuffer();

  CommonVertex vertex_[VertexCount];
  VertexArray<CommonVertex> vertex_array_;
  bool need_update_ = true;
};

template <int VertexCount>
inline CommonVertexDrawable<VertexCount>::CommonVertexDrawable(
    const GLID<IndexBuffer>& ibo) {
  vertex_array_.vbo = VertexBuffer::Gen();
  vertex_array_.ibo = ibo;

  VertexArray<CommonVertex>::Init(vertex_array_);
}

template <int VertexCount>
inline CommonVertexDrawable<VertexCount>::~CommonVertexDrawable() {
  VertexArray<CommonVertex>::Uninit(vertex_array_);

  VertexBuffer::Del(vertex_array_.vbo);
}

template <int VertexCount>
inline void CommonVertexDrawable<VertexCount>::Draw() {
  if (need_update_) {
    UpdateBuffer();
    need_update_ = false;
  }
}

template <int VertexCount>
inline void CommonVertexDrawable<VertexCount>::UpdateBuffer() {
  VertexBuffer::Bind(vertex_array_.vbo);
  VertexBuffer::BufferData(sizeof(vertex_), &vertex_, GL_DYNAMIC_DRAW);
  VertexBuffer::Unbind();
}

}  // namespace renderer

#endif  // !RENDERER_DRAW_DRAWABLE_H_
