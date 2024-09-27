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

RenderDevice::RenderDevice() {  // Init generic texture
  generic_size_ = base::Vec2i(32, 32);
  generic_texture_ =
      bgfx::createTexture2D(generic_size_.x, generic_size_.y, false, 1,
                            bgfx::TextureFormat::RGBA8, BGFX_TEXTURE_BLIT_DST);

  framebuffer_size_ = base::Vec2i(64, 64);
  bgfx::TextureHandle framebuffer_target = bgfx::createTexture2D(
      framebuffer_size_.x, framebuffer_size_.y, false, 1,
      bgfx::TextureFormat::RGBA8, BGFX_TEXTURE_BLIT_DST | BGFX_TEXTURE_RT);
  common_framebuffer_ = bgfx::createFrameBuffer(1, &framebuffer_target, true);

  quad_array_indices_ = new QuadArrayIndices();
  generic_quad_ = std::make_unique<QuadDrawable>(quad_array_indices_);

  pipelines_ = std::make_unique<Pipeline>();
}

RenderDevice::~RenderDevice() {
  if (bgfx::isValid(generic_texture_))
    bgfx::destroy(generic_texture_);

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

  return std::unique_ptr<RenderDevice>(new RenderDevice);
}

bgfx::TextureHandle RenderDevice::GetGenericTexture(const base::Vec2i& size,
                                                    base::Vec2i* out_size,
                                                    bool clamp_size) {
  if (clamp_size) {
    if (generic_size_.x == size.x && generic_size_.y == size.y) {
      if (out_size)
        *out_size = generic_size_;
      return generic_texture_;
    }
  } else {
    if (generic_size_.x >= size.x && generic_size_.y >= size.y) {
      if (out_size)
        *out_size = generic_size_;
      return generic_texture_;
    }
  }

  base::Vec2i new_size = size;
  if (!clamp_size) {
    new_size.x = std::max(size.x, generic_size_.x);
    new_size.y = std::max(size.y, generic_size_.y);
  }

  if (bgfx::isValid(generic_texture_))
    bgfx::destroy(generic_texture_);

  generic_texture_ =
      bgfx::createTexture2D(new_size.x, new_size.y, false, 1,
                            bgfx::TextureFormat::RGBA8, BGFX_TEXTURE_BLIT_DST);
  generic_size_ = new_size;
  if (out_size)
    *out_size = generic_size_;

  return generic_texture_;
}

bgfx::FrameBufferHandle RenderDevice::EnsureCommonFramebuffer(
    const base::Vec2i& size,
    base::Vec2i* out_size,
    bool clamp_size) {
  if (clamp_size) {
    if (framebuffer_size_.x == size.x && framebuffer_size_.y == size.y) {
      if (out_size)
        *out_size = framebuffer_size_;
      return common_framebuffer_;
    }
  } else {
    if (framebuffer_size_.x >= size.x && framebuffer_size_.y >= size.y) {
      if (out_size)
        *out_size = framebuffer_size_;
      return common_framebuffer_;
    }
  }

  base::Vec2i new_size = size;
  if (!clamp_size) {
    new_size.x = std::max(size.x, framebuffer_size_.x);
    new_size.y = std::max(size.y, framebuffer_size_.y);
  }

  if (bgfx::isValid(common_framebuffer_))
    bgfx::destroy(common_framebuffer_);

  bgfx::TextureHandle render_target = bgfx::createTexture2D(
      new_size.x, new_size.y, false, 1, bgfx::TextureFormat::RGBA8,
      BGFX_TEXTURE_BLIT_DST | BGFX_TEXTURE_RT);
  common_framebuffer_ = bgfx::createFrameBuffer(1, &render_target, true);

  framebuffer_size_ = new_size;
  if (out_size)
    *out_size = framebuffer_size_;

  return common_framebuffer_;
}

scoped_refptr<QuadArrayIndices> RenderDevice::quad_indices() {
  return quad_array_indices_;
}

}  // namespace renderer
