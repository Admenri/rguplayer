// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/common/tilemap_common.h"

#include "renderer/thread/thread_manager.h"

namespace content {

void TilemapFlashLayer::InitFlashLayer(int tile_size) {
  quads_ =
      std::make_unique<renderer::QuadDrawableArray<renderer::CommonVertex>>();
  tile_size_ = tile_size;
}

void TilemapFlashLayer::BeforeComposite() {
  if (buffer_need_update_) {
    UpdateBuffer();
    buffer_need_update_ = false;
  }
}

void TilemapFlashLayer::Composite(const base::Vec2i offset, float alpha) {
  renderer::GSM.states.blend_func.Push(renderer::GLBlendType::Addition);

  auto& shader = renderer::GSM.shaders()->flash_tile;
  shader.Bind();
  shader.SetProjectionMatrix(renderer::GSM.states.viewport.Current().Size());
  shader.SetTransOffset(offset);
  shader.SetAlpha(alpha);

  quads_->Draw();

  renderer::GSM.states.blend_func.Pop();
}

bool TilemapFlashLayer::SampleFlashColor(base::Vec4& out, int x, int y) const {
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

void TilemapFlashLayer::UpdateBuffer() {
  if (!flashdata_)
    return;

  std::vector<renderer::CommonVertex> vertices;
  for (int x = 0; x < flash_viewport_.width; ++x) {
    for (int y = 0; y < flash_viewport_.height; ++y) {
      base::Vec4 color;

      if (!SampleFlashColor(color, x + flash_viewport_.x,
                            y + flash_viewport_.y))
        continue;

      base::Rect posRect(x * tile_size_, y * tile_size_, tile_size_,
                         tile_size_);

      renderer::CommonVertex v[4];
      renderer::QuadSetPositionRect(v, posRect);
      renderer::QuadSetColor(v, -1, color);

      for (size_t i = 0; i < 4; ++i)
        vertices.push_back(v[i]);
    }
  }

  if (vertices.empty())
    return;

  size_t quad_size = vertices.size() / 4;
  quads_->Resize(quad_size);
  memcpy(quads_->vertices().data(), vertices.data(),
         vertices.size() * sizeof(renderer::CommonVertex));
  quads_->Update();

  renderer::GSM.quad_ibo()->EnsureSize(quad_size);
}

}  // namespace content
