#ifndef RENDERER_GL_FRAME_BUFFER_TEXTURE_H_
#define RENDERER_GL_FRAME_BUFFER_TEXTURE_H_

#include <memory>

#include "gpu/gles2/gles_context.h"

namespace renderer {

class FrameBufferTexture {
 public:
  FrameBufferTexture(std::shared_ptr<gpu::GLES2CommandContext> context);
  virtual ~FrameBufferTexture();

  FrameBufferTexture(const FrameBufferTexture&) = delete;
  FrameBufferTexture& operator=(const FrameBufferTexture&) = delete;

  GLuint GetTexture() { return texture_; }

  void Bind();
  void Unbind();

 private:
  std::shared_ptr<gpu::GLES2CommandContext> context_;
  GLuint texture_;
  GLuint frame_buffer_;
};

}  // namespace renderer

#endif  // RENDERER_GL_FRAME_BUFFER_TEXTURE_H_