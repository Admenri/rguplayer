// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/common/tileutils.h"

#include "renderer/device/render_device.h"

namespace content {

TilemapFlashLayer::TilemapFlashLayer(
    scoped_refptr<renderer::QuadArrayIndices> indices,
    int tile_size)
    : quads_(std::make_unique<renderer::QuadArray>(indices)),
      tile_size_(tile_size) {}

void TilemapFlashLayer::BeforeComposite() {
  if (buffer_need_update_) {
    UpdateBuffer();
    buffer_need_update_ = false;
  }
}

void TilemapFlashLayer::Composite(GraphicsHost* host,
                                  CompositeTargetInfo* target_info,
                                  const base::Vec2i offset,
                                  float alpha) {
  auto& shader = host->device()->pipelines().alphaflat;

  base::Vec4 ualpha;
  ualpha.x = alpha;
  target_info->encoder->setUniform(shader.Alpha(), &ualpha);

  if (target_info->render_scissor.enable)
    target_info->SetScissorRegion(target_info->render_scissor.region);

  target_info->encoder->setState(
      BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A |
      renderer::MakeColorBlendState(renderer::BlendType::Addition));
  quads_->Draw(target_info->encoder, shader.GetProgram(),
               target_info->render_view);
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

  std::vector<renderer::GeometryVertexLayout::Data> vertices;
  for (int x = 0; x < flash_viewport_.width; ++x) {
    for (int y = 0; y < flash_viewport_.height; ++y) {
      base::Vec4 color;

      if (!SampleFlashColor(color, x + flash_viewport_.x,
                            y + flash_viewport_.y))
        continue;

      base::Rect posRect(x * tile_size_, y * tile_size_, tile_size_,
                         tile_size_);

      renderer::GeometryVertexLayout::Data v[4];
      renderer::GeometryVertexLayout::SetPosition(v, posRect);
      renderer::GeometryVertexLayout::SetColor(v, color);

      for (size_t i = 0; i < 4; ++i)
        vertices.push_back(v[i]);
    }
  }

  if (vertices.empty())
    return;

  size_t quad_size = vertices.size() / 4;
  quads_->Resize(quad_size);
  memcpy(quads_->vertices().data(), vertices.data(),
         vertices.size() * sizeof(renderer::GeometryVertexLayout::Data));
  quads_->Update();
}

}  // namespace content
