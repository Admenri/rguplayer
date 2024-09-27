// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/graphics.h"

#include "components/fpslimiter/fpslimiter.h"
#include "content/public/bitmap.h"
#include "content/public/disposable.h"
#include "content/public/input.h"

#include "SDL3/SDL_timer.h"

namespace content {

Graphics::Graphics(base::WeakPtr<ui::Widget> window,
                   filesystem::Filesystem* io,
                   scoped_refptr<Profile> config,
                   const base::Vec2i& initial_resolution)
    : resolution_(initial_resolution),
      screen_buffer_(BGFX_INVALID_HANDLE),
      frozen_snapshot_(BGFX_INVALID_HANDLE),
      config_(config),
      window_(window),
      frozen_(false),
      brightness_(255),
      frame_count_(0),
      vsync_(0),
      frame_rate_(config_->content_version() >= APIVersion::RGSS2 ? 60 : 40),
      fps_manager_(std::make_unique<fpslimiter::FPSLimiter>(frame_rate_)),
      io_(io) {
  // Initialize root viewport
  viewport_rect().rect = initial_resolution;
  viewport_rect().has_scissor = false;

  // Create root font manager
  static_font_manager_ =
      std::make_unique<ScopedFontData>(io_, config_->default_font_path());

  // Reset fps manager
  fps_manager_->Reset();

  // Create render device
  bgfx::Init init_param;
  init_param.type = bgfx::RendererType::Direct3D12;
  init_param.resolution.reset = BGFX_RESET_VSYNC;
  init_param.resolution.format = bgfx::TextureFormat::RGBA8;
  init_param.resolution.width = resolution_.x;
  init_param.resolution.height = resolution_.y;
  device_ = renderer::RenderDevice::CreateContext(init_param, window_);

  // Create renderer buffer
  screen_quad_ =
      std::make_unique<renderer::QuadDrawable>(device_->quad_indices());
  RebuildScreenBufferInternal();
}

Graphics::~Graphics() {}

int Graphics::GetBrightness() const {
  return brightness_;
}

void Graphics::SetBrightness(int brightness) {
  brightness = std::clamp(brightness, 0, 255);
  brightness_ = brightness;
}

void Graphics::Wait(int duration) {
  for (int i = 0; i < duration; ++i) {
    Update();
  }
}

scoped_refptr<Bitmap> Graphics::SnapToBitmap() {
  scoped_refptr<Bitmap> snap = new Bitmap(this, resolution_.x, resolution_.y);

  return snap;
}

void Graphics::FadeOut(int duration) {}

void Graphics::FadeIn(int duration) {}

void Graphics::Update() {}

void Graphics::ResizeScreen(const base::Vec2i& resolution) {
  if (resolution_ == resolution)
    return;
  resolution_ = resolution;
  RebuildScreenBufferInternal();
}

void Graphics::Reset() {
  /* Reset freeze */
  frozen_ = false;

  /* Disposed all elements */
  for (auto it = disposable_elements_.tail(); it != disposable_elements_.end();
       it = it->previous()) {
    it->value()->Dispose();
  }

  /* Reset attribute */
  SetFrameRate(api_version() >= APIVersion::RGSS2 ? 60 : 40);
  SetBrightness(255);
  FrameReset();
}

void Graphics::Freeze() {
  if (frozen_)
    return;

  frozen_ = true;
}

void Graphics::Transition(int duration,
                          scoped_refptr<Bitmap> trans_bitmap,
                          int vague) {
  if (!frozen_)
    return;

  SetBrightness(255);
  vague = std::clamp<int>(vague, 1, 256);
}

void Graphics::SetFrameRate(int rate) {
  rate = std::max(rate, 10);
  fps_manager_->SetFrameRate(rate);
  frame_rate_ = rate;
}

int Graphics::GetFrameRate() const {
  return frame_rate_;
}

void Graphics::SetFrameCount(int64_t count) {
  frame_count_ = count;
}

int Graphics::GetFrameCount() const {
  return frame_count_;
}

void Graphics::FrameReset() {
  fps_manager_->Reset();
}

uint64_t Graphics::GetWindowHandle() {
  uint64_t window_handle = 0;
#if defined(OS_WIN)
  window_handle = (uint64_t)SDL_GetPointerProperty(
      SDL_GetWindowProperties(window_->AsSDLWindow()),
      SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
#else
  // TODO: other platform window handle
#endif
  return window_handle;
}

void Graphics::ResizeWindow(int width, int height) {
  auto* win = window_->AsSDLWindow();

  SDL_SetWindowSize(win, width, height);
  SDL_SetWindowPosition(win, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

bool Graphics::GetFullscreen() {
  return window_->IsFullscreen();
}

void Graphics::SetFullscreen(bool fullscreen) {
  window_->SetFullscreen(fullscreen);
}

void Graphics::SetVSync(int interval) {
  vsync_ = interval;
  bgfx::reset(resolution_.x, resolution_.y, interval ? BGFX_RESET_VSYNC : 0);
}

int Graphics::GetVSync() {
  return vsync_;
}

bool Graphics::GetFrameSkip() {
  return config_->allow_frame_skip();
}

void Graphics::SetFrameSkip(bool skip) {
  config_->allow_frame_skip() = skip;
}

int Graphics::GetDisplayWidth() {
  return window_->GetSize().x;
}

int Graphics::GetDisplayHeight() {
  return window_->GetSize().y;
}

void Graphics::SetDrawableOffset(const base::Vec2i& offset) {
  viewport_rect().rect.x = offset.x;
  viewport_rect().rect.y = offset.y;
  DrawableParent::NotifyViewportRectChanged();
}

base::Vec2i Graphics::GetDrawableOffset() {
  return viewport_rect().rect.Position();
}

void Graphics::RebuildScreenBufferInternal() {
  if (bgfx::isValid(screen_buffer_))
    bgfx::destroy(screen_buffer_);
  if (bgfx::isValid(frozen_snapshot_))
    bgfx::destroy(frozen_snapshot_);

  bgfx::TextureHandle screen_texture =
      bgfx::createTexture2D(resolution_.x, resolution_.y, false, 1,
                            bgfx::TextureFormat::RGBA8, BGFX_TEXTURE_RT);
  bgfx::TextureHandle snapshot_texture =
      bgfx::createTexture2D(resolution_.x, resolution_.y, false, 1,
                            bgfx::TextureFormat::RGBA8, BGFX_TEXTURE_RT);

  screen_buffer_ = bgfx::createFrameBuffer(1, &screen_texture, true);
  frozen_snapshot_ = bgfx::createFrameBuffer(1, &snapshot_texture, true);
}

void Graphics::FrameProcessInternal() {
  /* Control frame delay */
  fps_manager_->Delay();

  /* Increase frame render count */
  ++frame_count_;

  /* Update average fps */
  UpdateAverageFPSInternal();
}

void Graphics::AddDisposable(Disposable* disp) {
  disposable_elements_.Append(disp->disposable_link());
}

void Graphics::RemoveDisposable(Disposable* disp) {
  disp->disposable_link()->RemoveFromList();
}

void Graphics::UpdateAverageFPSInternal() {}

void Graphics::UpdateWindowViewportInternal() {
  window_size_ = window_->GetSize();

  float window_ratio = static_cast<float>(window_size_.x) / window_size_.y;
  float screen_ratio = static_cast<float>(resolution_.x) / resolution_.y;

  display_viewport_.width = window_size_.x;
  display_viewport_.height = window_size_.y;

  if (screen_ratio > window_ratio)
    display_viewport_.height = display_viewport_.width / screen_ratio;
  else if (screen_ratio < window_ratio)
    display_viewport_.width = display_viewport_.height * screen_ratio;

  display_viewport_.x = (window_size_.x - display_viewport_.width) / 2.0f;
  display_viewport_.y = (window_size_.y - display_viewport_.height) / 2.0f;
}

}  // namespace content
