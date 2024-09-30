// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "renderer/device/render_device.h"

#include "base/debug/logging.h"

#include "SDL3/SDL_video.h"

namespace renderer {

struct DeviceHandler : public bgfx::CallbackI {
  void fatal(const char* _filePath,
             uint16_t _line,
             bgfx::Fatal::Enum _code,
             const char* _str) override {
    printf("[Renderer] %s (%d)\n", _str, _code);
  }

  void traceVargs(const char* _filePath,
                  uint16_t _line,
                  const char* _format,
                  va_list _argList) override {}

  void profilerBegin(const char* _name,
                     uint32_t _abgr,
                     const char* _filePath,
                     uint16_t _line) override {}

  void profilerBeginLiteral(const char* _name,
                            uint32_t _abgr,
                            const char* _filePath,
                            uint16_t _line) override {}

  void profilerEnd() override {}

  uint32_t cacheReadSize(uint64_t _id) override { return 0; }

  bool cacheRead(uint64_t _id, void* _data, uint32_t _size) override {
    return false;
  }

  void cacheWrite(uint64_t _id, const void* _data, uint32_t _size) override {}

  void screenShot(const char* _filePath,
                  uint32_t _width,
                  uint32_t _height,
                  uint32_t _pitch,
                  const void* _data,
                  uint32_t _size,
                  bool _yflip) override {}

  void captureBegin(uint32_t _width,
                    uint32_t _height,
                    uint32_t _pitch,
                    bgfx::TextureFormat::Enum _format,
                    bool _yflip) override {}

  void captureEnd() override {}

  void captureFrame(const void* _data, uint32_t _size) override {}
};

static DeviceHandler g_device_handler;

RenderDevice::RenderDevice(base::WeakPtr<ui::Widget> screen) : screen_(screen) {
  GetGenericTexture(base::Vec2i(64, 64), nullptr);
  EnsureCommonFramebuffer(base::Vec2i(64, 64), nullptr);

  quad_array_indices_ = new QuadArrayIndices();
  generic_quad_ = std::make_unique<QuadDrawable>(quad_array_indices_);

  pipelines_ = std::make_unique<Pipeline>();
}

RenderDevice::~RenderDevice() {
  bgfx::destroy(generic_texture_.handle);
  bgfx::destroy(common_framebuffer_.handle);

  generic_quad_.reset();
  quad_array_indices_.reset();
  pipelines_.reset();

  bgfx::shutdown();
}

std::unique_ptr<RenderDevice> RenderDevice::CreateContext(
    const bgfx::Init& init_param,
    base::WeakPtr<ui::Widget> target) {
  SDL_PropertiesID window_prop = SDL_GetWindowProperties(target->AsSDLWindow());

  bgfx::Init renderer_init(init_param);
  renderer_init.callback = &g_device_handler;
#if defined(OS_WIN)
  renderer_init.platformData.nwh = SDL_GetPointerProperty(
      window_prop, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
#else
#error unsupport platform
#endif

  if (!bgfx::init(renderer_init))
    return nullptr;

  return std::unique_ptr<RenderDevice>(new RenderDevice(target));
}

void RenderDevice::GetGenericTexture(const base::Vec2i& size,
                                     Texture* out,
                                     bool clamp_size) {
  auto& tex = generic_texture_;
  if (clamp_size) {
    if (tex.size.x == size.x && tex.size.y == size.y) {
      if (out)
        *out = tex;
      return;
    }
  } else {
    if (tex.size.x >= size.x && tex.size.y >= size.y) {
      if (out)
        *out = tex;
      return;
    }
  }

  base::Vec2i new_size = size;
  if (!clamp_size) {
    new_size.x = std::max(size.x, generic_texture_.size.x);
    new_size.y = std::max(size.y, generic_texture_.size.y);
  }

  if (bgfx::isValid(tex.handle))
    bgfx::destroy(tex.handle);
  tex.handle =
      bgfx::createTexture2D(new_size.x, new_size.y, false, 1,
                            bgfx::TextureFormat::RGBA8, BGFX_TEXTURE_BLIT_DST);
  tex.size = new_size;
  if (out)
    *out = tex;
}

void RenderDevice::EnsureCommonFramebuffer(const base::Vec2i& size,
                                           Framebuffer* out,
                                           bool clamp_size) {
  auto& fbo = common_framebuffer_;
  if (clamp_size) {
    if (fbo.size.x == size.x && fbo.size.y == size.y) {
      if (out)
        *out = fbo;
      return;
    }
  } else {
    if (fbo.size.x >= size.x && fbo.size.y >= size.y) {
      if (out)
        *out = fbo;
      return;
    }
  }

  base::Vec2i new_size = size;
  if (!clamp_size) {
    new_size.x = std::max(size.x, fbo.size.x);
    new_size.y = std::max(size.y, fbo.size.y);
  }

  if (bgfx::isValid(fbo.handle))
    bgfx::destroy(fbo.handle);

  bgfx::TextureHandle render_target = bgfx::createTexture2D(
      new_size.x, new_size.y, false, 1, bgfx::TextureFormat::RGBA8,
      BGFX_TEXTURE_BLIT_DST | BGFX_TEXTURE_RT);
  fbo.handle = bgfx::createFrameBuffer(1, &render_target, true);

  fbo.size = new_size;
  if (out)
    *out = fbo;
}

scoped_refptr<QuadArrayIndices> RenderDevice::quad_indices() {
  return quad_array_indices_;
}

void RenderDevice::BindRenderView(bgfx::ViewId render_view,
                                  const base::Rect& viewport,
                                  bgfx::FrameBufferHandle render_target,
                                  std::optional<uint32_t> clear) {
  float proj_mat[16];
  renderer::MakeProjectionMatrix(proj_mat, viewport.Size());
  bgfx::setViewRect(render_view, viewport.x, viewport.y, viewport.width,
                    viewport.height);
  bgfx::setViewTransform(render_view, nullptr, proj_mat);
  bgfx::setViewFrameBuffer(render_view, render_target);
  bgfx::setViewClear(render_view,
                     clear.has_value() ? BGFX_CLEAR_COLOR : BGFX_CLEAR_NONE,
                     clear.has_value() ? *clear : 0);
}

}  // namespace renderer
