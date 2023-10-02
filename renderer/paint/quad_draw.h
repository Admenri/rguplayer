// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RENDERER_PAINT_QUAD_DRAW_H_
#define RENDERER_PAINT_QUAD_DRAW_H_

#include "base/memory/ref_counted.h"
#include "gpu/gl_forward.h"
#include "renderer/buffer/vertex_buffer.h"

namespace renderer {

class QuadIndicesBuffer : public base::RefCountedThreadSafe<QuadIndicesBuffer> {
 public:
  QuadIndicesBuffer(scoped_refptr<gpu::GLES2CommandContext> context);
  virtual ~QuadIndicesBuffer();

  QuadIndicesBuffer(const QuadIndicesBuffer&) = delete;
  QuadIndicesBuffer& operator=(const QuadIndicesBuffer) = delete;

  void Bind();
  void Unbind();

 private:
  friend class base::RefCountedThreadSafe<QuadIndicesBuffer>;
  scoped_refptr<gpu::GLES2CommandContext> context_;
  GLuint indices_buffer_;
};

class QuadDrawable {
 public:
  QuadDrawable(scoped_refptr<QuadIndicesBuffer> indices_buffer,
               scoped_refptr<gpu::GLES2CommandContext> context);
  virtual ~QuadDrawable() = default;

  QuadDrawable(const QuadDrawable&) = delete;
  QuadDrawable& operator=(const QuadDrawable&) = delete;

  void SetPosition(const base::RectF& rect);
  void SetTexcoord(const base::RectF& rect);
  void SetColor(int index, const base::Vec4& color);

  void Draw();

 private:
  scoped_refptr<gpu::GLES2CommandContext> context_;
  scoped_refptr<QuadIndicesBuffer> indices_buffer_;

  std::unique_ptr<GLVertexData<CommonVertex>> vertex_data_;

  CommonVertex vertex_[4];
  bool need_update_ = false;
};

}  // namespace renderer

#endif  // RENDERER_PAINT_QUAD_DRAW_H_