// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_COMMON_TILEMAP_COMMON_H_
#define CONTENT_COMMON_TILEMAP_COMMON_H_

#include "content/public/table.h"
#include "renderer/quad/quad_array.h"
#include "renderer/thread/thread_manager.h"

namespace content {

inline int vwrap(int value, int range) {
  int res = value % range;
  return res < 0 ? res + range : res;
};

inline base::Vec2i vwrap(const base::Vec2i& value, int range) {
  return base::Vec2i(vwrap(value.x, range), vwrap(value.y, range));
}

inline int16_t TableGetWrapped(scoped_refptr<content::Table> t,
                               int x,
                               int y,
                               int z = 0) {
  return t->Get(vwrap(x, t->GetXSize()), vwrap(y, t->GetYSize()), z);
}

inline int16_t TableGetFlag(scoped_refptr<content::Table> t, int x) {
  if (!t)
    return 0;

  if (x < 0 || x >= t->GetXSize())
    return 0;

  return t->At(x);
}

class TilemapFlashLayer {
 public:
  TilemapFlashLayer()
      : quads_(new renderer::QuadDrawableArray<renderer::CommonVertex>()) {}
  ~TilemapFlashLayer() {}

  TilemapFlashLayer(const TilemapFlashLayer&) = delete;
  TilemapFlashLayer& operator=(const TilemapFlashLayer&) = delete;

  scoped_refptr<Table> GetFlashData() const { return flashdata_; }
  void SetFlashData(scoped_refptr<Table> data) {
    if (data == flashdata_)
      return;

    flashdata_ = data;
    buffer_need_update_ = true;

    observer_ = data->AddObserver(base::BindRepeating(
        &TilemapFlashLayer::SetUpdateBuffer, base::Unretained(this)));
  }

  void SetViewport(const base::Rect& viewport) {
    flash_viewport_ = viewport;
    buffer_need_update_ = true;
  }

  void BeforeComposite() {
    if (buffer_need_update_) {
      UpdateBuffer();
      buffer_need_update_ = false;
    }
  }

  void Composite(const base::Vec2i offset, float alpha) {
    renderer::GSM.states.blend_func.Push(renderer::GLBlendType::Addition);

    auto& shader = renderer::GSM.shaders->flash_tile;
    shader.Bind();
    shader.SetProjectionMatrix(renderer::GSM.states.viewport.Current().Size());
    shader.SetAlpha(alpha);

    quads_->Draw();

    renderer::GSM.states.blend_func.Pop();
  }

 private:
  void SetUpdateBuffer() { buffer_need_update_ = true; }
  bool SampleFlashColor(base::Vec4& out, int x, int y) const {
    int16_t packed = TableGetWrapped(flashdata_, x, y);

    if (!packed)
      return false;

    const float max = 0xF;

    float b = ((packed & 0x000F) >> 0) / max;
    float g = ((packed & 0x00F0) >> 4) / max;
    float r = ((packed & 0x0F00) >> 8) / max;

    out = base::Vec4(r, g, b, 1);

    return true;
  }

  void UpdateBuffer() {
    if (!flashdata_)
      return;

    std::vector<renderer::CommonVertex> vertices;
    for (int x = 0; x < flash_viewport_.width; ++x) {
      for (int y = 0; y < flash_viewport_.height; ++y) {
        base::Vec4 color;

        if (!SampleFlashColor(color, x + flash_viewport_.x,
                              y + flash_viewport_.y))
          continue;

        base::RectF posRect(x * 32, y * 32, 32, 32);

        renderer::CommonVertex v[4];
        renderer::QuadSetPositionRect(v, posRect);
        renderer::QuadSetColor(v, -1, color);

        for (size_t i = 0; i < 4; ++i)
          vertices.push_back(v[i]);
      }
    }

    if (vertices.empty())
      return;

    int quad_size = vertices.size() / 4;
    quads_->Resize(quad_size);
    memcpy(&quads_->vertices()[0], &vertices[0],
           vertices.size() * sizeof(renderer::CommonVertex));
    quads_->Update();

    renderer::GSM.quad_ibo->EnsureSize(quad_size);
  }

  scoped_refptr<Table> flashdata_;
  std::unique_ptr<renderer::QuadDrawableArray<renderer::CommonVertex>> quads_;
  base::Rect flash_viewport_;
  bool buffer_need_update_ = false;

  base::CallbackListSubscription observer_;
};

}  // namespace content

#endif  // !CONTENT_COMMON_TILEMAP_COMMON_H_
