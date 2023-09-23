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

  Bind();
  context_->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_2D, texture_, 0);
  Unbind();
}

FrameBufferTexture::~FrameBufferTexture() {
  context_->glDeleteFramebuffers(1, &frame_buffer_);
  context_->glDeleteTextures(1, &texture_);
}

void FrameBufferTexture::Bind() {
  context_->glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_);
}

void FrameBufferTexture::Unbind() {
  context_->glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

}  // namespace renderer