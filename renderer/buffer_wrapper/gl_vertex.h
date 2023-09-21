#ifndef RENDERER_BUFFER_WRAPPER_GL_VERTEX_H_
#define RENDERER_BUFFER_WRAPPER_GL_VERTEX_H_

#include <memory>

#include "base/math/math.h"
#include "gpu/gles2/gles_context.h"

namespace renderer {

struct CommonVertex {
  base::Vec2 position;
  base::Vec2 texcoord;
  base::Vec4 color;

  CommonVertex() = default;
};

/// <summary>
/// Universal vertex type traits
/// </summary>

enum ShaderLocation : GLenum {
  Position = 0,
  TexCoord,
  Color,
};

struct GLVertexAttribute {
  ShaderLocation location;
  size_t size;
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
  GLVertexData(std::shared_ptr<gpu::GLES2CommandContext> context);
  virtual ~GLVertexData();

  GLVertexData(const GLVertexData&) = delete;
  GLVertexData& operator=(const GLVertexData&) = delete;

  void Bind();
  void Unbind();
  void UpdateVertex(const VertexT* raw_data, size_t size);

 private:
  std::shared_ptr<gpu::GLES2CommandContext> context_;

  GLuint vertex_buffer_;
};

}  // namespace renderer

#endif  // RENDERER_BUFFER_WRAPPER_GL_VERTEX_H_