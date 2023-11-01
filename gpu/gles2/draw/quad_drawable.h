// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_GLES2_DRAW_QUAD_DRAWABLE_H_
#define GPU_GLES2_DRAW_QUAD_DRAWABLE_H_

#include <vector>

#include "base/math/math.h"
#include "gpu/gles2/gsm/gles_gsm.h"
#include "gpu/gles2/meta/gles_meta.h"
#include "gpu/gles2/vertex/vertex_array.h"

namespace gpu {

template <int VertexCount>
class CommonVertexDrawable {
 public:
  CommonVertexDrawable(const GLID<IndexBuffer>& ibo);
  virtual ~CommonVertexDrawable();

  CommonVertexDrawable(const CommonVertexDrawable&) = delete;
  CommonVertexDrawable& operator=(const CommonVertexDrawable&) = delete;

  virtual void Draw();

 protected:
  virtual void UpdateBuffer();

  CommonVertex vertex_[VertexCount];
  VertexArray<CommonVertex> vertex_array_;
  bool need_update_ = true;
};

template <int VertexCount>
inline CommonVertexDrawable<VertexCount>::CommonVertexDrawable(
    const GLID<IndexBuffer>& ibo) {
  vertex_array_.vbo = VertexBuffer::Gen();
  vertex_array_.ibo = ibo;

  VertexArray<CommonVertex>::Init(vertex_array_);
}

template <int VertexCount>
inline CommonVertexDrawable<VertexCount>::~CommonVertexDrawable() {
  VertexArray<CommonVertex>::Uninit(vertex_array_);

  VertexBuffer::Del(vertex_array_.vbo);
}

template <int VertexCount>
inline void CommonVertexDrawable<VertexCount>::Draw() {
  if (need_update_) {
    UpdateBuffer();
    need_update_ = false;
  }
}

template <int VertexCount>
inline void CommonVertexDrawable<VertexCount>::UpdateBuffer() {
  VertexBuffer::Bind(vertex_array_.vbo);
  VertexBuffer::BufferData(sizeof(vertex_), &vertex_, GL_DYNAMIC_DRAW);
  VertexBuffer::Unbind();
}

class GeometryDrawable final : public CommonVertexDrawable<3> {
 public:
  GeometryDrawable();

  void SetPosition(int index, const base::Vec2& pos);
  void SetTexCoord(int index, const base::Vec2& texcoord);
  void SetColor(int index, const base::Vec4& color);

  void Draw() override;
};

class QuadIndexBuffer {
 public:
  QuadIndexBuffer();
  ~QuadIndexBuffer();

  QuadIndexBuffer(const QuadIndexBuffer&) = delete;
  QuadIndexBuffer& operator=(const QuadIndexBuffer&) = delete;

  void EnsureSize(size_t count);

  GLID<IndexBuffer> ibo;

 private:
  std::vector<uint16_t> buffer;
};

class QuadDrawable final : public CommonVertexDrawable<4> {
 public:
  QuadDrawable();

  void SetPositionRect(const base::RectF& pos);
  void SetTexCoordRect(const base::RectF& texcoord);
  void SetColor(int index, const base::Vec4& color);

  void Draw() override;

 private:
  base::RectF position_cache_;
  base::RectF texCoord_cache_;
};

template <typename V>
static void QuadSetPositionRect(V* vert, const base::RectF& pos) {
  int i = -1;
  vert[++i].position = base::Vec2(pos.x, pos.y);
  vert[++i].position = base::Vec2(pos.x + pos.width, pos.y);
  vert[++i].position = base::Vec2(pos.x + pos.width, pos.y + pos.height);
  vert[++i].position = base::Vec2(pos.x, pos.y + pos.height);
}

template <typename V>
static void QuadSetTexCoordRect(V* vert, const base::RectF& texcoord) {
  int i = -1;
  vert[++i].texCoord = base::Vec2(texcoord.x, texcoord.y);
  vert[++i].texCoord = base::Vec2(texcoord.x + texcoord.width, texcoord.y);
  vert[++i].texCoord =
      base::Vec2(texcoord.x + texcoord.width, texcoord.y + texcoord.height);
  vert[++i].texCoord = base::Vec2(texcoord.x, texcoord.y + texcoord.height);
}

struct Blt {
  static void BeginScreen(const base::Vec2i& resolution);
  static void BeginDraw(TextureFrameBuffer& dest_tfb);
  static void TexSource(TextureFrameBuffer& src_tfb);
  static void EndDraw(const base::Rect& src_rect, const base::Rect& dest_rect);
};

}  // namespace gpu

#endif  // GPU_GLES2_DRAW_QUAD_DRAWABLE_H_