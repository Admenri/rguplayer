#include "renderer/buffer_wrapper/gl_vertex.h"

namespace renderer {

#define VertexAttrDefine(t)                                               \
  const GLVertexAttribute* VertexAttributeTraits<CommonVertex>::attr = t; \
  size_t VertexAttributeTraits<CommonVertex>::attr_size =                 \
      sizeof(t) / sizeof(t[0]);

const GLVertexAttribute CommonVertexAttr[] = {
    {
        ShaderLocation::Position,
        2,
        GL_FLOAT,
        reinterpret_cast<const GLvoid*>(offsetof(CommonVertex, position)),
    },
    {
        ShaderLocation::TexCoord,
        2,
        GL_FLOAT,
        reinterpret_cast<const GLvoid*>(offsetof(CommonVertex, texcoord)),
    },
    {
        ShaderLocation::Color,
        4,
        GL_FLOAT,
        reinterpret_cast<const GLvoid*>(offsetof(CommonVertex, color)),
    },
};
VertexAttrDefine(CommonVertexAttr);

/// <summary>
/// Universal vertex type traits
/// </summary>

template <typename VertexT>
GLVertexData<VertexT>::GLVertexData(
    std::shared_ptr<gpu::GLES2CommandContext> context)
    : context_(context) {
  context_->glGenBuffers(1, &vertex_buffer_);
}

template <typename VertexT>
GLVertexData<VertexT>::~GLVertexData() {
  context_->glDeleteBuffers(1, &vertex_buffer_);
}

template <typename VertexT>
void GLVertexData<VertexT>::Bind() {
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
void GLVertexData<VertexT>::Unbind() {
  context_->glBindBuffer(GL_ARRAY_BUFFER, 0);

  for (size_t i = 0; i < VertexAttributeTraits<CommonVertex>::attr_size; i++) {
    context_->glDisableVertexAttribArray(
        VertexAttributeTraits<CommonVertex>::attr[i].location);
  }
}

template <typename VertexT>
void GLVertexData<VertexT>::UpdateVertex(const VertexT* raw_data, size_t size) {
  context_->glBufferData(GL_ARRAY_BUFFER, size * sizeof(VertexT), raw_data,
                         GL_DYNAMIC_DRAW);
}

}  // namespace renderer
