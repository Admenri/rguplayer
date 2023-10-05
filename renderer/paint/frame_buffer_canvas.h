// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RENDERER_PAINT_FRAME_BUFFER_CANVAS_H_
#define RENDERER_PAINT_FRAME_BUFFER_CANVAS_H_

#include <memory>

#include "base/math/math.h"
#include "base/memory/ref_counted.h"
#include "gpu/gl_forward.h"
#include "renderer/compositor/renderer_cc.h"

namespace renderer {

class CCLayer;

class GLTexture : public base::RefCountedThreadSafe<GLTexture> {
 public:
  GLTexture(scoped_refptr<gpu::GLES2CommandContext> context);
  virtual ~GLTexture();

  GLTexture(const GLTexture&) = delete;
  GLTexture& operator=(const GLTexture&) = delete;

  void SetSize(const base::Vec2i& size) { size_ = size; }
  base::Vec2i GetSize() { return size_; }

  void AllocEmpty();
  void BufferData(const void* data, GLenum format);
  void BufferData(const base::Vec4i& bounds, const void* data, GLenum format);

  void Activate(uint16_t tex_unit);
  GLuint GetTextureRaw() { return texture_; }
  void Bind();
  void Unbind();

  void SetTextureWrap(GLint mode);
  void SetTextureFilter(GLint mode);

 private:
  scoped_refptr<gpu::GLES2CommandContext> context_;
  base::Vec2i size_;
  GLuint texture_;
};

class GLFrameBuffer {
 public:
  GLFrameBuffer(scoped_refptr<gpu::GLES2CommandContext> context);
  virtual ~GLFrameBuffer();

  GLFrameBuffer(const GLFrameBuffer&) = delete;
  GLFrameBuffer& operator=(const GLFrameBuffer&) = delete;

  void SetRenderTarget(scoped_refptr<GLTexture> target);
  scoped_refptr<GLTexture> GetRenderTarget() { return texture_; }

  void ReadPixels(const base::Vec4i& bounds, GLenum format, GLenum data_type,
                  void* pixels);

  void Bind();
  void Unbind();

  void Clear();
  void Clear(const base::Vec4& color);

  static void BltBegin(CCLayer* cc, GLFrameBuffer* target,
                       const base::Vec2i& size);
  static void BltSource(CCLayer* cc, scoped_refptr<GLTexture> target);
  static void BltClear(CCLayer* cc);
  static void BltEnd(CCLayer* cc, GLFrameBuffer* target,
                     const base::Rect& src_rect, const base::Rect& dst_rect);

 private:
  scoped_refptr<gpu::GLES2CommandContext> context_;
  scoped_refptr<GLTexture> texture_;
  GLuint frame_buffer_;
};

class DoubleFrameBuffer {
 public:
  struct Buffer {
    scoped_refptr<GLTexture> texture;
    std::unique_ptr<GLFrameBuffer> frame_buffer;
  };

  DoubleFrameBuffer(scoped_refptr<gpu::GLES2CommandContext> context,
                    const base::Vec2i& size);
  virtual ~DoubleFrameBuffer();

  DoubleFrameBuffer(const DoubleFrameBuffer&) = delete;
  DoubleFrameBuffer& operator=(const DoubleFrameBuffer&) = delete;

  void Resize(const base::Vec2i& size);
  void Swap();

  Buffer* GetFrontend() { return &frames_[0]; }
  Buffer* GetBackend() { return &frames_[1]; }

 private:
  Buffer frames_[2];
};

}  // namespace renderer

#endif  // RENDERER_PAINT_FRAME_BUFFER_CANVAS_H_