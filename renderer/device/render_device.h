// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RENDERER_DEVICE_RENDER_DEVICE_H_
#define RENDERER_DEVICE_RENDER_DEVICE_H_

#include "bx/math.h"
#include "renderer/drawable/input_layout.h"
#include "renderer/drawable/quad_drawable.h"
#include "renderer/pipeline/render_pipeline.h"
#include "ui/widget/widget.h"

#include <stdint.h>
#include <memory>

namespace renderer {

inline void MakeProjectionMatrix(float* out, const base::Vec2& size) {
  const bool origin_bottom = bgfx::getCaps()->originBottomLeft;
  const bool homogeneous = bgfx::getCaps()->homogeneousDepth;

  const float aa = 2.0f / size.x;
  const float bb = (origin_bottom ? 2.0f : -2.0f) / size.y;
  const float cc = homogeneous ? 2.0f : 1.0f;
  const float dd = -1.0f;
  const float ee = origin_bottom ? -1.0f : 1.0f;
  const float ff = homogeneous ? -1.0f : 0.0f;

  memset(out, 0, sizeof(float) * 16);
  out[0] = aa;
  out[5] = bb;
  out[10] = cc;
  out[12] = dd;
  out[13] = ee;
  out[14] = ff;
  out[15] = 1.0f;
}

class RenderDevice final {
 public:
  using Pipeline = struct {
    BaseShader base;
    TexbltShader texblt;
    BaseColorShader color;
    HueShader hue;
  };

  ~RenderDevice();

  RenderDevice(const RenderDevice&) = delete;
  RenderDevice& operator=(const RenderDevice&) = delete;

  static std::unique_ptr<RenderDevice> CreateContext(
      const bgfx::Init& init_param,
      base::WeakPtr<ui::Widget> target);

  bgfx::TextureHandle GetGenericTexture(const base::Vec2i& size,
                                        base::Vec2i* out_size,
                                        bool clamp_size = false);
  bgfx::FrameBufferHandle EnsureCommonFramebuffer(const base::Vec2i& size,
                                                  base::Vec2i* out_size,
                                                  bool clamp_size = false);

  Pipeline& pipelines() { return *pipelines_; }
  scoped_refptr<QuadArrayIndices> quad_indices();
  QuadDrawable* common_quad() { return generic_quad_.get(); }

 private:
  RenderDevice();

  scoped_refptr<QuadArrayIndices> quad_array_indices_;
  std::unique_ptr<Pipeline> pipelines_;

  bgfx::TextureHandle generic_texture_;
  base::Vec2i generic_size_;

  bgfx::FrameBufferHandle common_framebuffer_;
  base::Vec2i framebuffer_size_;

  std::unique_ptr<QuadDrawable> generic_quad_;
};

}  // namespace renderer

#endif  //! RENDERER_DEVICE_RENDER_DEVICE_H_
