// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "renderer/quad/quad_drawable.h"

#include "base/math/math.h"
#include "renderer/thread/thread_manager.h"

#include <array>

namespace renderer {

static const std::array<uint16_t, 6> index_template = {0, 1, 2, 2, 3, 0};

GeometryDrawable::GeometryDrawable()
    : CommonVertexDrawable(GLID<IndexBuffer>()) {}

void GeometryDrawable::SetPosition(int index, const base::Vec2& pos) {
  CommonVertex& vert = vertex_[std::clamp(index, 0, 2)];

  if (vert.position == pos)
    return;

  vert.position = pos;
  need_update_ = true;
}

void GeometryDrawable::SetTexCoord(int index, const base::Vec2& texcoord) {
  CommonVertex& vert = vertex_[std::clamp(index, 0, 2)];

  if (vert.texCoord == texcoord)
    return;

  vert.texCoord = texcoord;
  need_update_ = true;
}

void GeometryDrawable::SetColor(int index, const base::Vec4& color) {
  CommonVertex& vert = vertex_[std::clamp(index, 0, 2)];

  if (vert.color == color)
    return;

  vert.color = color;
  need_update_ = true;
}

void GeometryDrawable::Draw() {
  CommonVertexDrawable::Draw();

  VertexArray<CommonVertex>::Bind(vertex_array_);
  GL.DrawArrays(GL_TRIANGLES, 0, 3);
  VertexArray<CommonVertex>::Unbind();
}

QuadIndexBuffer::QuadIndexBuffer() {
  ibo = IndexBuffer::Gen();
}

QuadIndexBuffer::~QuadIndexBuffer() {
  IndexBuffer::Del(ibo);
}

void QuadIndexBuffer::EnsureSize(size_t count) {
  if (buffer.size() >= count * 6)
    return;

  size_t begin = buffer.size() / 6;
  buffer.reserve(count * 6);
  for (size_t i = begin; i < count; ++i)
    for (size_t j = 0; j < 6; ++j)
      buffer.push_back(static_cast<uint16_t>(i * 4 + index_template[j]));

  IndexBuffer::Bind(ibo);
  IndexBuffer::BufferData(buffer.size() * sizeof(uint16_t), &buffer[0]);
  IndexBuffer::Unbind();
}

QuadDrawable::QuadDrawable() : CommonVertexDrawable(GSM.quad_ibo->ibo) {}

void QuadDrawable::SetPositionRect(const base::RectF& pos) {
  if (position_cache_ == pos)
    return;

  int i = 0;
  vertex_[i++].position = base::Vec2(pos.x, pos.y);
  vertex_[i++].position = base::Vec2(pos.x + pos.width, pos.y);
  vertex_[i++].position = base::Vec2(pos.x + pos.width, pos.y + pos.height);
  vertex_[i++].position = base::Vec2(pos.x, pos.y + pos.height);
  need_update_ = true;

  position_cache_ = pos;
}

void QuadDrawable::SetTexCoordRect(const base::RectF& texcoord) {
  if (texCoord_cache_ == texcoord)
    return;

  int i = 0;
  vertex_[i++].texCoord = base::Vec2(texcoord.x, texcoord.y);
  vertex_[i++].texCoord = base::Vec2(texcoord.x + texcoord.width, texcoord.y);
  vertex_[i++].texCoord =
      base::Vec2(texcoord.x + texcoord.width, texcoord.y + texcoord.height);
  vertex_[i++].texCoord = base::Vec2(texcoord.x, texcoord.y + texcoord.height);
  need_update_ = true;

  texCoord_cache_ = texcoord;
}

void QuadDrawable::SetColor(int index, const base::Vec4& color) {
  if (index == -1) {
    int i = 0;
    vertex_[i++].color = color;
    vertex_[i++].color = color;
    vertex_[i++].color = color;
    vertex_[i++].color = color;
    need_update_ = true;
    return;
  }

  CommonVertex& vert = vertex_[std::clamp(index, 0, 3)];
  if (vert.color == color)
    return;
  vert.color = color;
  need_update_ = true;
}

void QuadDrawable::Draw() {
  CommonVertexDrawable::Draw();

  VertexArray<CommonVertex>::Bind(vertex_array_);
  GL.DrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
  VertexArray<CommonVertex>::Unbind();
}

void Blt::BeginScreen(const base::Rect& rect) {
  FrameBuffer::Unbind();

  auto& shader = GSM.shaders->base;

  GSM.states.viewport.Push(rect);
  GSM.states.blend.Push(false);

  shader.Bind();
  shader.SetProjectionMatrix(rect.Size());
  shader.SetTransOffset(base::Vec2i());
}

void Blt::BeginDraw(TextureFrameBuffer& dest_tfb) {
  FrameBuffer::Bind(dest_tfb.fbo);

  auto size = base::Vec2i(dest_tfb.width, dest_tfb.height);
  auto& shader = GSM.shaders->base;

  GSM.states.viewport.Push(size);
  GSM.states.blend.Push(false);

  shader.Bind();
  shader.SetProjectionMatrix(size);
  shader.SetTransOffset(base::Vec2i());
}

void Blt::TexSource(TextureFrameBuffer& src_tfb) {
  auto& shader = GSM.shaders->base;

  shader.SetTexture(src_tfb.tex);
  shader.SetTextureSize(base::Vec2i(src_tfb.width, src_tfb.height));
}

void Blt::BltDraw(const base::RectF& src_rect, const base::RectF& dest_rect) {
  auto* quad = GSM.common_quad.get();

  quad->SetPositionRect(dest_rect);
  quad->SetTexCoordRect(src_rect);
  quad->Draw();
}

void Blt::BltDraw(const base::Rect& src_rect, const base::Rect& dest_rect) {
  auto* quad = GSM.common_quad.get();

  quad->SetPositionRect(dest_rect);
  quad->SetTexCoordRect(src_rect);
  quad->Draw();
}

void Blt::EndDraw() {
  GSM.states.viewport.Pop();
  GSM.states.blend.Pop();
}

}  // namespace renderer
