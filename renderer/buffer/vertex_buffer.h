// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RENDERER_BUFFER_VERTEX_BUFFER_H_
#define RENDERER_BUFFER_VERTEX_BUFFER_H_

#include "base/math/math.h"
#include "base/memory/ref_counted.h"
#include "gpu/gl_forward.h"
#include "gpu/gles2/shader/shader_manager.h"

namespace renderer {

struct CommonVertex {
  base::Vec2 position;
  base::Vec2 texcoord;
  base::Vec4 color;
};

struct GLVertexAttribute {
  gpu::ShaderLocation location;
  int size;
  GLenum type;
  const GLvoid* offset;
};

template <typename VertexT>
struct VertexAttributeTraits {
  static const GLVertexAttribute* attr;
  static size_t attr_size;
};

template <typename VertexT>
class GLVertexData {
 public:
  GLVertexData(scoped_refptr<gpu::GLES2CommandContext> context);
  virtual ~GLVertexData();

  GLVertexData(const GLVertexData&) = delete;
  GLVertexData& operator=(const GLVertexData&) = delete;

  void Bind();
  void Unbind();
  void UpdateVertex(const VertexT* raw_data, size_t size);

 private:
  scoped_refptr<gpu::GLES2CommandContext> context_;

  GLuint vertex_buffer_;
};

template <typename VertexT>
inline GLVertexData<VertexT>::GLVertexData(
    scoped_refptr<gpu::GLES2CommandContext> context)
    : context_(context) {
  context_->glGenBuffers(1, &vertex_buffer_);
}

template <typename VertexT>
inline GLVertexData<VertexT>::~GLVertexData() {
  context_->glDeleteBuffers(1, &vertex_buffer_);
}

template <typename VertexT>
inline void GLVertexData<VertexT>::Bind() {
  context_->glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);

  for (size_t i = 0; i < VertexAttributeTraits<CommonVertex>::attr_size; i++) {
    context_->glEnableVertexAttribArray(
        VertexAttributeTraits<CommonVertex>::attr[i].location);
    context_->glVertexAttribPointer(
        VertexAttributeTraits<CommonVertex>::attr[i].location,
        VertexAttributeTraits<CommonVertex>::attr[i].size,
        VertexAttributeTraits<CommonVertex>::attr[i].type, GL_FALSE,
        sizeof(VertexT), VertexAttributeTraits<CommonVertex>::attr[i].offset);
  }
}

template <typename VertexT>
inline void GLVertexData<VertexT>::Unbind() {
  context_->glBindBuffer(GL_ARRAY_BUFFER, 0);

  for (size_t i = 0; i < VertexAttributeTraits<CommonVertex>::attr_size; i++) {
    context_->glDisableVertexAttribArray(
        VertexAttributeTraits<CommonVertex>::attr[i].location);
  }
}

template <typename VertexT>
inline void GLVertexData<VertexT>::UpdateVertex(const VertexT* raw_data,
                                                size_t size) {
  context_->glBufferData(GL_ARRAY_BUFFER, size * sizeof(VertexT), raw_data,
                         GL_DYNAMIC_DRAW);
}

}  // namespace renderer

#endif  // RENDERER_BUFFER_VERTEX_BUFFER_H_