// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef RENDERER_QUAD_QUAD_DRAWABLE_H_
#define RENDERER_QUAD_QUAD_DRAWABLE_H_

#include "base/math/math.h"
#include "renderer/draw/drawable.h"
#include "renderer/vertex/vertex_set.h"

#include <algorithm>

namespace renderer {

class QuadIndexBuffer {
 public:
  QuadIndexBuffer();
  ~QuadIndexBuffer();

  QuadIndexBuffer(const QuadIndexBuffer&) = delete;
  QuadIndexBuffer& operator=(const QuadIndexBuffer&) = delete;

  void EnsureSize(size_t count);

  GLID<IndexBuffer> GetBuffer() { return ibo_; }

 private:
  GLID<IndexBuffer> ibo_;
  std::vector<uint16_t> buffer_;
};

class QuadDrawable final : public Drawable<CommonVertex, 4> {
 public:
  QuadDrawable();

  QuadDrawable(const QuadDrawable&) = delete;
  QuadDrawable& operator=(const QuadDrawable&) = delete;

  void SetPositionRect(const base::RectF& pos);
  void SetTexCoordRect(const base::RectF& texcoord);
  void SetColor(int index = -1, const base::Vec4& color = base::Vec4());

  void Draw();

 private:
  base::RectF position_cache_;
  base::RectF texcoord_cache_;
};

template <typename V>
static void QuadSetPositionRect(V* vert, const base::RectF& pos) {
  int i = -1;
  vert[++i].position = base::Vec4(pos.x, pos.y, 0, 1);
  vert[++i].position = base::Vec4(pos.x + pos.width, pos.y, 0, 1);
  vert[++i].position = base::Vec4(pos.x + pos.width, pos.y + pos.height, 0, 1);
  vert[++i].position = base::Vec4(pos.x, pos.y + pos.height, 0, 1);
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

template <typename V>
static void QuadSetColor(V* vert,
                         int index = -1,
                         const base::Vec4& color = base::Vec4()) {
  if (index == -1) {
    int i = -1;
    vert[++i].color = color;
    vert[++i].color = color;
    vert[++i].color = color;
    vert[++i].color = color;
    return;
  }

  vert[std::clamp(index, 0, 3)].color = color;
}

template <typename V>
static int QuadSetTexPosRect(V* vert,
                             const base::RectF& texcoord,
                             const base::RectF& pos) {
  QuadSetTexCoordRect(vert, texcoord);
  QuadSetPositionRect(vert, pos);

  return 1;
}

struct Blt {
  static void BeginScreen(const base::Rect& rect);
  static void BeginDraw(const TextureFrameBuffer& dest_tfb);
  static void TexSource(const TextureFrameBuffer& src_tfb);
  static void BltDraw(const base::Rect& src_rect,
                      const base::Rect& dest_rect,
                      bool smooth = false);
  static void BltDraw(const base::RectF& src_rect,
                      const base::RectF& dest_rect,
                      bool smooth = false);
};

}  // namespace renderer

#endif  // !RENDERER_QUAD_QUAD_DRAWABLE_H_