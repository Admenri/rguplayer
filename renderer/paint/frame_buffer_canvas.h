// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RENDERER_PAINT_FRAME_BUFFER_CANVAS_H_
#define RENDERER_PAINT_FRAME_BUFFER_CANVAS_H_

#include <memory>

#include "base/math/math.h"
#include "gpu/gl_forward.h"

namespace renderer {

class FrameBufferTexture {
 public:
  FrameBufferTexture(scoped_refptr<gpu::GLES2CommandContext> context);
  virtual ~FrameBufferTexture();

  FrameBufferTexture(const FrameBufferTexture&) = delete;
  FrameBufferTexture& operator=(const FrameBufferTexture&) = delete;

  GLuint GetTexture() { return texture_; }

  void BindFrame();
  void UnbindFrame();

  void BindTexture();
  void UnbindTexture();

  void Clear();

  void Alloc(const base::Vec2i& size);
  void BufferData(const base::Vec2i& size, const void* data, GLenum format);
  void BufferData(const base::Vec4i& bounds, const void* data, GLenum format);

  base::Vec2i& GetSize() { return size_; }

 private:
  scoped_refptr<gpu::GLES2CommandContext> context_;
  GLuint texture_;
  GLuint frame_buffer_;
  base::Vec2i size_;
};

}  // namespace renderer

#endif  // RENDERER_PAINT_FRAME_BUFFER_CANVAS_H_