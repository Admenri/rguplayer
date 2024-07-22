// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "renderer/quad/quad_drawable.h"

#include "base/math/math.h"
#include "renderer/thread/thread_manager.h"

#include <array>

namespace renderer {

static const std::array<uint16_t, 6> kQuadIndexTemplate = {0, 1, 2, 2, 3, 0};

QuadIndexBuffer::QuadIndexBuffer() {
  ibo_ = IndexBuffer::Gen();
}

QuadIndexBuffer::~QuadIndexBuffer() {
  IndexBuffer::Del(ibo_);
}

void QuadIndexBuffer::EnsureSize(size_t count) {
  if (buffer_.size() >= count * 6)
    return;

  size_t begin = buffer_.size() / 6;
  buffer_.reserve(count * 6);
  for (size_t i = begin; i < count; ++i)
    for (size_t j = 0; j < 6; ++j)
      buffer_.push_back(static_cast<uint16_t>(i * 4 + kQuadIndexTemplate[j]));

  IndexBuffer::Bind(ibo_);
  IndexBuffer::BufferData(buffer_.size() * sizeof(uint16_t), buffer_.data());
  IndexBuffer::Unbind();
}

QuadDrawable::QuadDrawable() : Drawable(GSM.quad_ibo()->GetBuffer()) {}

void QuadDrawable::SetPositionRect(const base::RectF& pos) {
  if (position_cache_ == pos)
    return;
  position_cache_ = pos;

  int i = 0;
  vertices()[i++].position = base::Vec4(pos.x, pos.y, 0, 1);
  vertices()[i++].position = base::Vec4(pos.x + pos.width, pos.y, 0, 1);
  vertices()[i++].position =
      base::Vec4(pos.x + pos.width, pos.y + pos.height, 0, 1);
  vertices()[i++].position = base::Vec4(pos.x, pos.y + pos.height, 0, 1);
  NotifyUpdate();
}

void QuadDrawable::SetTexCoordRect(const base::RectF& texcoord) {
  if (texcoord_cache_ == texcoord)
    return;
  texcoord_cache_ = texcoord;

  int i = 0;
  vertices()[i++].texCoord = base::Vec2(texcoord.x, texcoord.y);
  vertices()[i++].texCoord =
      base::Vec2(texcoord.x + texcoord.width, texcoord.y);
  vertices()[i++].texCoord =
      base::Vec2(texcoord.x + texcoord.width, texcoord.y + texcoord.height);
  vertices()[i++].texCoord =
      base::Vec2(texcoord.x, texcoord.y + texcoord.height);
  NotifyUpdate();
}

void QuadDrawable::SetColor(int index, const base::Vec4& color) {
  if (index == -1) {
    int i = 0;
    vertices()[i++].color = color;
    vertices()[i++].color = color;
    vertices()[i++].color = color;
    vertices()[i++].color = color;
  } else {
    index = std::clamp(index, 0, 3);
    CommonVertex& vert = vertices()[index];
    if (vert.color == color)
      return;
    vert.color = color;
  }

  NotifyUpdate();
}

void QuadDrawable::Draw() {
  Drawable::UpdateBuffer();

  VertexArray<CommonVertex>::Bind(vertex_array());
  GL.DrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
  VertexArray<CommonVertex>::Unbind();
}

void Blt::BeginScreen(const base::Rect& rect) {
  GL.BindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void Blt::BeginDraw(const TextureFrameBuffer& dest_tfb) {
  GL.BindFramebuffer(GL_DRAW_FRAMEBUFFER, dest_tfb.fbo.gl);
}

void Blt::TexSource(const TextureFrameBuffer& src_tfb) {
  return GL.BindFramebuffer(GL_READ_FRAMEBUFFER, src_tfb.fbo.gl);
}

void Blt::BltDraw(const base::RectF& src_rect,
                  const base::RectF& dest_rect,
                  bool smooth) {
  GL.BlitFramebuffer(src_rect.x, src_rect.y, src_rect.x + src_rect.width,
                     src_rect.y + src_rect.height, dest_rect.x, dest_rect.y,
                     dest_rect.x + dest_rect.width,
                     dest_rect.y + dest_rect.height, GL_COLOR_BUFFER_BIT,
                     smooth ? GL_LINEAR : GL_NEAREST);
}

void Blt::BltDraw(const base::Rect& src_rect,
                  const base::Rect& dest_rect,
                  bool smooth) {
  GL.BlitFramebuffer(src_rect.x, src_rect.y, src_rect.x + src_rect.width,
                     src_rect.y + src_rect.height, dest_rect.x, dest_rect.y,
                     dest_rect.x + dest_rect.width,
                     dest_rect.y + dest_rect.height, GL_COLOR_BUFFER_BIT,
                     smooth ? GL_LINEAR : GL_NEAREST);
}

}  // namespace renderer
