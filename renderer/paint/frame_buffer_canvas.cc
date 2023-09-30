// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "renderer/paint/frame_buffer_canvas.h"

namespace renderer {

FrameBufferTexture::FrameBufferTexture(
    scoped_refptr<gpu::GLES2CommandContext> context)
    : context_(context) {
  context_->glGenTextures(1, &texture_);
  context_->glGenFramebuffers(1, &frame_buffer_);

  BindFrame();
  context_->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_2D, texture_, 0);
  UnbindFrame();
}

FrameBufferTexture::~FrameBufferTexture() {
  context_->glDeleteFramebuffers(1, &frame_buffer_);
  context_->glDeleteTextures(1, &texture_);
}

void FrameBufferTexture::BindFrame() {
  context_->glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_);
}

void FrameBufferTexture::UnbindFrame() {
  context_->glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBufferTexture::BindTexture() {
  context_->glBindTexture(GL_TEXTURE_2D, texture_);
}

void FrameBufferTexture::UnbindTexture() {
  context_->glBindTexture(GL_TEXTURE_2D, 0);
}

void FrameBufferTexture::Clear() {
  context_->glClearColor(0.f, 0.f, 0.f, 0.f);
  context_->glClear(GL_COLOR_BUFFER_BIT);
}

void FrameBufferTexture::Alloc(const base::Vec2i& size) {
  size_ = size;
  context_->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, 0);
}

void FrameBufferTexture::BufferData(const base::Vec2i& size, const void* data,
                                    GLenum format) {
  size_ = size;
  context_->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, format,
                         GL_UNSIGNED_BYTE, data);
}

void FrameBufferTexture::BufferData(const base::Vec4i& bounds, const void* data,
                                    GLenum format) {
  context_->glTexSubImage2D(GL_TEXTURE_2D, 0, bounds.x, bounds.y, bounds.z,
                            bounds.w, format, GL_UNSIGNED_BYTE, data);
}

}  // namespace renderer