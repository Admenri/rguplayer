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
      buffer.push_back(static_cast<uint16_t>(i * 4 + kQuadIndexTemplate[j]));

  IndexBuffer::Bind(ibo);
  IndexBuffer::BufferData(buffer.size() * sizeof(uint16_t), buffer.data());
  IndexBuffer::Unbind();
}

QuadDrawable::QuadDrawable() : CommonVertexDrawable(GSM.quad_ibo()->ibo) {}

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
  if (GL.BlitFramebuffer)
    return GL.BindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

  FrameBuffer::Unbind();
  GSM.states.viewport.Push(rect);
  auto& shader = GSM.shaders()->base;
  shader.Bind();
  shader.SetProjectionMatrix(rect.Size());
  shader.SetTransOffset(base::Vec2i());
}

void Blt::BeginDraw(const TextureFrameBuffer& dest_tfb) {
  if (GL.BlitFramebuffer)
    return GL.BindFramebuffer(GL_DRAW_FRAMEBUFFER, dest_tfb.fbo.gl);

  FrameBuffer::Bind(dest_tfb.fbo);
  auto size = base::Vec2i(dest_tfb.width, dest_tfb.height);
  GSM.states.viewport.Push(size);
  auto& shader = GSM.shaders()->base;
  shader.Bind();
  shader.SetProjectionMatrix(size);
  shader.SetTransOffset(base::Vec2i());
}

void Blt::TexSource(const TextureFrameBuffer& src_tfb) {
  if (GL.BlitFramebuffer)
    return GL.BindFramebuffer(GL_READ_FRAMEBUFFER, src_tfb.fbo.gl);

  auto& shader = GSM.shaders()->base;
  shader.SetTexture(src_tfb.tex);
  shader.SetTextureSize(base::Vec2i(src_tfb.width, src_tfb.height));
}

void Blt::BltDraw(const base::RectF& src_rect,
                  const base::RectF& dest_rect,
                  bool smooth) {
  if (GL.BlitFramebuffer)
    return GL.BlitFramebuffer(
        src_rect.x, src_rect.y, src_rect.x + src_rect.width,
        src_rect.y + src_rect.height, dest_rect.x, dest_rect.y,
        dest_rect.x + dest_rect.width, dest_rect.y + dest_rect.height,
        GL_COLOR_BUFFER_BIT, smooth ? GL_LINEAR : GL_NEAREST);

  if (smooth)
    Texture::SetFilter(GL_LINEAR);
  auto* quad = GSM.common_quad();
  GSM.states.blend.Push(false);
  quad->SetPositionRect(dest_rect);
  quad->SetTexCoordRect(src_rect);
  quad->Draw();
  GSM.states.blend.Pop();
  if (smooth)
    Texture::SetFilter(GL_NEAREST);
}

void Blt::BltDraw(const base::Rect& src_rect,
                  const base::Rect& dest_rect,
                  bool smooth) {
  if (GL.BlitFramebuffer)
    return GL.BlitFramebuffer(
        src_rect.x, src_rect.y, src_rect.x + src_rect.width,
        src_rect.y + src_rect.height, dest_rect.x, dest_rect.y,
        dest_rect.x + dest_rect.width, dest_rect.y + dest_rect.height,
        GL_COLOR_BUFFER_BIT, smooth ? GL_LINEAR : GL_NEAREST);

  if (smooth)
    Texture::SetFilter(GL_LINEAR);
  auto* quad = GSM.common_quad();
  GSM.states.blend.Push(false);
  quad->SetPositionRect(dest_rect);
  quad->SetTexCoordRect(src_rect);
  quad->Draw();
  GSM.states.blend.Pop();
  if (smooth)
    Texture::SetFilter(GL_NEAREST);
}

void Blt::EndDraw() {
  if (GL.BlitFramebuffer)
    return;
  GSM.states.viewport.Pop();
}

}  // namespace renderer
