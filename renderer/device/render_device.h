// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RENDERER_DEVICE_RENDER_DEVICE_H_
#define RENDERER_DEVICE_RENDER_DEVICE_H_

#include "bgfx/defines.h"
#include "bx/math.h"
#include "renderer/drawable/input_layout.h"
#include "renderer/drawable/quad_drawable.h"
#include "renderer/pipeline/render_pipeline.h"
#include "ui/widget/widget.h"

#include <stdint.h>
#include <memory>

namespace renderer {

enum class BlendType {
  KeepDestAlpha = -1,

  Normal = 0,
  Addition = 1,
  Substraction = 2
};

struct Texture {
  bgfx::TextureHandle handle = BGFX_INVALID_HANDLE;
  base::Vec2i size;
};

struct Framebuffer {
  bgfx::FrameBufferHandle handle = BGFX_INVALID_HANDLE;
  base::Vec2i size;
};

inline uint64_t MakeColorBlendState(BlendType type) {
  switch (type) {
    case BlendType::KeepDestAlpha:
      return BGFX_STATE_BLEND_EQUATION(BGFX_STATE_BLEND_EQUATION_ADD) |
             BGFX_STATE_BLEND_FUNC_SEPARATE(
                 BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA,
                 BGFX_STATE_BLEND_ZERO, BGFX_STATE_BLEND_ONE);
    default:
    case BlendType::Normal:
      return BGFX_STATE_BLEND_EQUATION(BGFX_STATE_BLEND_EQUATION_ADD) |
             BGFX_STATE_BLEND_FUNC_SEPARATE(
                 BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA,
                 BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_INV_SRC_ALPHA);
    case BlendType::Addition:
      return BGFX_STATE_BLEND_EQUATION(BGFX_STATE_BLEND_EQUATION_ADD) |
             BGFX_STATE_BLEND_FUNC_SEPARATE(
                 BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_ONE,
                 BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_ONE);
    case BlendType::Substraction:
      return BGFX_STATE_BLEND_EQUATION(BGFX_STATE_BLEND_EQUATION_REVSUB) |
             BGFX_STATE_BLEND_FUNC_SEPARATE(
                 BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_ONE,
                 BGFX_STATE_BLEND_ZERO, BGFX_STATE_BLEND_ONE);
  }
}

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
    ViewportShader viewport;
    SpriteShader sprite;
    AlphaSpriteShader alphasprite;
    BaseSpriteShader basesprite;
  };

  ~RenderDevice();

  RenderDevice(const RenderDevice&) = delete;
  RenderDevice& operator=(const RenderDevice&) = delete;

  static std::unique_ptr<RenderDevice> CreateContext(
      const bgfx::Init& init_param,
      base::WeakPtr<ui::Widget> target);

  void GetGenericTexture(const base::Vec2i& size,
                         Texture* out,
                         bool clamp_size = false);
  void EnsureCommonFramebuffer(const base::Vec2i& size,
                               Framebuffer* out,
                               bool clamp_size = false);

  Pipeline& pipelines() { return *pipelines_; }
  scoped_refptr<QuadArrayIndices> quad_indices();
  QuadDrawable* common_quad() { return generic_quad_.get(); }
  base::WeakPtr<ui::Widget> window() { return screen_; }

  void BindRenderView(bgfx::ViewId render_view,
                      const base::Rect& viewport,
                      bgfx::FrameBufferHandle render_target,
                      uint32_t clear = 0);

 private:
  RenderDevice(base::WeakPtr<ui::Widget> screen);

  base::WeakPtr<ui::Widget> screen_;

  scoped_refptr<QuadArrayIndices> quad_array_indices_;
  std::unique_ptr<Pipeline> pipelines_;

  Texture generic_texture_;
  Framebuffer common_framebuffer_;

  std::unique_ptr<QuadDrawable> generic_quad_;
};

}  // namespace renderer

#endif  //! RENDERER_DEVICE_RENDER_DEVICE_H_
