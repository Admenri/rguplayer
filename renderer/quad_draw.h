#ifndef RENDERER_QUAD_DRAW_H_
#define RENDERER_QUAD_DRAW_H_

#include <memory>

#include "gpu/gles2/gles_context.h"
#include "renderer/buffer_wrapper/gl_vertex.h"

namespace renderer {

class QuadIndicesBuffer {
 public:
  QuadIndicesBuffer(std::shared_ptr<gpu::GLES2CommandContext> context);
  virtual ~QuadIndicesBuffer();

  QuadIndicesBuffer(const QuadIndicesBuffer&) = delete;
  QuadIndicesBuffer& operator=(const QuadIndicesBuffer) = delete;

  void Bind();
  void Unbind();

 private:
  std::shared_ptr<gpu::GLES2CommandContext> context_;
  GLuint indices_buffer_;
};

class QuadDrawable {
 public:
  QuadDrawable(std::shared_ptr<QuadIndicesBuffer> indices_buffer,
               std::shared_ptr<gpu::GLES2CommandContext> context);
  virtual ~QuadDrawable() = default;

  QuadDrawable(const QuadDrawable&) = delete;
  QuadDrawable& operator=(const QuadDrawable&) = delete;

  void SetPosition(const base::RectF& rect);
  void SetTexcoord(const base::RectF& rect);
  void SetColor(const base::Vec4& color);

  void Draw();

 private:
  std::shared_ptr<gpu::GLES2CommandContext> context_;
  std::unique_ptr<GLVertexData<CommonVertex>> vertex_data_;
  std::shared_ptr<QuadIndicesBuffer> indices_buffer_;

  CommonVertex vertex_[4];
  bool need_update_ = false;
};

}  // namespace renderer

#endif  // RENDERER_OGL_QUAD_DRAW_H_