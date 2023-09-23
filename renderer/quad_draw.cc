#include "renderer/quad_draw.h"

#include "base/debug/debugwriter.h"

namespace renderer {

/*
 *  About Triangle Element:
 *
 *  0-1
 *  |\|
 *  3-2
 *
 *  Draw index: 0 -> 1 -> 2 (1)
 *              2 -> 3 -> 0 (2)
 *              Total: 6 indices
 *                     2 triangles
 *
 */

static const GLbyte kQuadIndices[] = {
    0, 1, 2, 2, 3, 0,
};

QuadIndicesBuffer::QuadIndicesBuffer(
    std::shared_ptr<gpu::GLES2CommandContext> context) {
  context_->glGenBuffers(1, &indices_buffer_);

  Bind();
  context_->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kQuadIndices),
                         kQuadIndices, GL_STATIC_DRAW);
  Unbind();
}

QuadIndicesBuffer::~QuadIndicesBuffer() {
  context_->glDeleteBuffers(1, &indices_buffer_);
}

void QuadIndicesBuffer::Bind() {
  context_->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_buffer_);
}

void QuadIndicesBuffer::Unbind() {
  context_->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

QuadDrawable::QuadDrawable(std::shared_ptr<QuadIndicesBuffer> indices_buffer,
                           std::shared_ptr<gpu::GLES2CommandContext> context)
    : context_(context),
      vertex_data_(std::make_unique<GLVertexData<CommonVertex>>(context_)),
      indices_buffer_(indices_buffer) {
  vertex_data_->Bind();
  vertex_data_->UpdateVertex(nullptr, 4);
  vertex_data_->Unbind();
}

void QuadDrawable::SetPosition(const base::RectF& rect) {
  int i = 0;
  vertex_[i++].position = base::Vec2(rect.x, rect.y);
  vertex_[i++].position = base::Vec2(rect.x + rect.width, rect.y);
  vertex_[i++].position = base::Vec2(rect.x + rect.width, rect.y + rect.height);
  vertex_[i++].position = base::Vec2(rect.x, rect.y + rect.height);
}

void QuadDrawable::SetTexcoord(const base::RectF& rect) {
  int i = 0;
  vertex_[i++].texcoord = base::Vec2(rect.x, rect.y);
  vertex_[i++].texcoord = base::Vec2(rect.x + rect.width, rect.y);
  vertex_[i++].texcoord = base::Vec2(rect.x + rect.width, rect.y + rect.height);
  vertex_[i++].texcoord = base::Vec2(rect.x, rect.y + rect.height);
}

void QuadDrawable::SetColor(const base::Vec4& color) {
  for (size_t i = 0; i < 4; i++) vertex_[i].color = color;
}

void QuadDrawable::Draw() {
  if (need_update_) {
    need_update_ = false;

    vertex_data_->UpdateVertex(vertex_, 4);
  }

  indices_buffer_->Bind();
  vertex_data_->Bind();
  context_->glDrawElements(GL_TRIANGLES,
                           sizeof(kQuadIndices) / sizeof(kQuadIndices[0]),
                           GL_UNSIGNED_BYTE, kQuadIndices);
  vertex_data_->Unbind();
  indices_buffer_->Unbind();
}

}  // namespace renderer
