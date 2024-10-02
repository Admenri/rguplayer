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
    : screen_buffer_(BGFX_INVALID_HANDLE),
      frozen_snapshot_(BGFX_INVALID_HANDLE),
      profile_(config),
      frozen_(false),
      brightness_(255),
      frame_count_(0),
      vsync_(0),
      frame_rate_(profile_->content_version() >= APIVersion::RGSS2 ? 60 : 40),
      fps_manager_(std::make_unique<fpslimiter::FPSLimiter>(frame_rate_)),
      io_(io) {
  // Initialize root viewport
  viewport_rect().rect = initial_resolution;
  viewport_rect().has_scissor = false;

  // Create root font manager
  static_font_manager_ =
      std::make_unique<ScopedFontData>(io_, profile_->default_font_path());

  // Reset fps manager
  fps_manager_->Reset();

  // Create render device
  bgfx::Init init_param;
  init_param.type = bgfx::RendererType::Direct3D12;
  init_param.resolution.reset = BGFX_RESET_NONE;
  init_param.resolution.format = bgfx::TextureFormat::RGBA8;
  init_param.resolution.width = initial_resolution.x;
  init_param.resolution.height = initial_resolution.y;
  device_ = renderer::RenderDevice::CreateContext(init_param, window);

  // Create renderer buffer
  screen_quad_ =
      std::make_unique<renderer::QuadDrawable>(device_->quad_indices());

  // Create virtual screen buffer
  RebuildScreenBufferInternal(initial_resolution);
}

Graphics::~Graphics() {
  screen_quad_.reset();

  if (bgfx::isValid(screen_buffer_.handle))
    bgfx::destroy(screen_buffer_.handle);
  if (bgfx::isValid(frozen_snapshot_.handle))
    bgfx::destroy(frozen_snapshot_.handle);

  device_.reset();
}

int Graphics::GetBrightness() const {
  return brightness_;
}

void Graphics::SetBrightness(int brightness) {
  brightness = std::clamp(brightness, 0, 255);
  brightness_ = brightness;
}

void Graphics::Wait(int duration) {
  for (int i = 0; i < duration; ++i)
    Update();
}

scoped_refptr<Bitmap> Graphics::SnapToBitmap() {
  scoped_refptr<Bitmap> snap =
      new Bitmap(this, screen_buffer_.size.x, screen_buffer_.size.y);

  bgfx::ViewId render_view = 1;
  bgfx::Encoder* encoder = bgfx::begin();

  // Composite
  EncodeScreenDrawcallsInternal(encoder, &render_view);

  // Blit to bitmap
  encoder->blit(render_view, bgfx::getTexture(snap->GetHandle()), 0, 0,
                bgfx::getTexture(screen_buffer_.handle));

  // Submit to GPU
  bgfx::end(encoder);
  bgfx::frame();

  return snap;
}

void Graphics::FadeOut(int duration) {}

void Graphics::FadeIn(int duration) {}

void Graphics::Update() {
  if (!frozen_) {
    if (fps_manager_->RequireFrameSkip()) {
      if (profile_->allow_frame_skip()) {
        // Skip render frame
        return FrameProcessInternal();
      } else {
        // Reset frame interval diff
        fps_manager_->Reset();
      }
    }

    bgfx::ViewId render_view = 1;
    bgfx::Encoder* encoder = bgfx::begin();

    // Composite
    EncodeScreenDrawcallsInternal(encoder, &render_view);

    // Present
    PresentScreenBufferInternal(encoder, render_view);

    // Submit to GPU
    bgfx::end(encoder);
    bgfx::frame();
  }

  // Process frame delay
  FrameProcessInternal();
}

void Graphics::ResizeScreen(const base::Vec2i& resolution) {
  if (!(screen_buffer_.size == resolution))
    RebuildScreenBufferInternal(resolution);
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
  ResetFrame();
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

void Graphics::ResetFrame() {
  fps_manager_->Reset();
}

void Graphics::ResizeWindow(int width, int height) {
  auto* win = device()->window()->AsSDLWindow();

  SDL_SetWindowSize(win, width, height);
  SDL_SetWindowPosition(win, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

bool Graphics::GetFullscreen() {
  return device()->window()->IsFullscreen();
}

void Graphics::SetFullscreen(bool fullscreen) {
  device()->window()->SetFullscreen(fullscreen);
}

void Graphics::SetVSync(int interval) {
  vsync_ = interval;
  bgfx::reset(screen_buffer_.size.x, screen_buffer_.size.y,
              interval ? BGFX_RESET_VSYNC : BGFX_RESET_NONE);
}

int Graphics::GetVSync() {
  return vsync_;
}

bool Graphics::GetFrameSkip() {
  return profile_->allow_frame_skip();
}

void Graphics::SetFrameSkip(bool skip) {
  profile_->allow_frame_skip() = skip;
}

int Graphics::GetDisplayWidth() {
  return device()->window()->GetSize().x;
}

int Graphics::GetDisplayHeight() {
  return device()->window()->GetSize().y;
}

void Graphics::SetDrawableOffset(const base::Vec2i& offset) {
  viewport_rect().rect.x = offset.x;
  viewport_rect().rect.y = offset.y;
  DrawableParent::NotifyViewportRectChanged();
}

base::Vec2i Graphics::GetDrawableOffset() {
  return viewport_rect().rect.Position();
}

void Graphics::RebuildScreenBufferInternal(const base::Vec2i& resolution) {
  if (bgfx::isValid(screen_buffer_.handle))
    bgfx::destroy(screen_buffer_.handle);
  if (bgfx::isValid(frozen_snapshot_.handle))
    bgfx::destroy(frozen_snapshot_.handle);

  bgfx::TextureHandle screen_texture =
      bgfx::createTexture2D(resolution.x, resolution.y, false, 1,
                            bgfx::TextureFormat::RGBA8, BGFX_TEXTURE_RT);
  bgfx::TextureHandle snapshot_texture =
      bgfx::createTexture2D(resolution.x, resolution.y, false, 1,
                            bgfx::TextureFormat::RGBA8, BGFX_TEXTURE_RT);

  screen_buffer_.handle = bgfx::createFrameBuffer(1, &screen_texture, true);
  screen_buffer_.size = resolution;
  frozen_snapshot_.handle = bgfx::createFrameBuffer(1, &snapshot_texture, true);
  frozen_snapshot_.size = resolution;
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
  auto window_size = device()->window()->GetSize();

  float window_ratio = static_cast<float>(window_size.x) / window_size.y;
  float screen_ratio =
      static_cast<float>(screen_buffer_.size.x) / screen_buffer_.size.y;

  display_viewport_.width = window_size.x;
  display_viewport_.height = window_size.y;

  if (screen_ratio > window_ratio)
    display_viewport_.height = display_viewport_.width / screen_ratio;
  else if (screen_ratio < window_ratio)
    display_viewport_.width = display_viewport_.height * screen_ratio;

  display_viewport_.x = (window_size.x - display_viewport_.width) / 2.0f;
  display_viewport_.y = (window_size.y - display_viewport_.height) / 2.0f;
}

void Graphics::EncodeScreenDrawcallsInternal(bgfx::Encoder* encoder,
                                             bgfx::ViewId* render_view) {
  // Execute prepare stage
  DrawableParent::PrepareComposite(encoder, render_view);

  // Composite stage
  device()->BindRenderView(*render_view, screen_buffer_.size,
                           screen_buffer_.handle, 0x000000ff);

  CompositeTargetInfo target_info;
  target_info.encoder = encoder;
  target_info.render_target = &screen_buffer_;
  target_info.render_view = *render_view;
  target_info.render_scissor.enable = false;
  target_info.render_scissor.region = screen_buffer_.size;
  target_info.render_scissor.cache = UINT16_MAX;

  // Execute composite
  DrawableParent::Composite(&target_info);

  *render_view = target_info.render_view;
}

void Graphics::PresentScreenBufferInternal(bgfx::Encoder* encoder,
                                           bgfx::ViewId render_view) {
  UpdateWindowViewportInternal();

  device()->window()->GetMouseState().resolution = screen_buffer_.size;
  device()->window()->GetMouseState().screen_offset =
      display_viewport_.Position();
  device()->window()->GetMouseState().screen = display_viewport_.Size();

  base::Vec2i window_size(bgfx::getStats()->width, bgfx::getStats()->height);
  device()->BindRenderView(render_view, window_size, BGFX_INVALID_HANDLE,
                           0x000000ff);

  auto& shader = device()->pipelines().base;

  base::Vec4 offset_size =
      base::MakeVec4(base::Vec2(), base::MakeInvert(screen_buffer_.size));
  encoder->setUniform(shader.OffsetTexSize(), &offset_size);
  encoder->setTexture(0, shader.Texture(),
                      bgfx::getTexture(screen_buffer_.handle));

  base::Rect target_rect = display_viewport_;
  if (bgfx::getCaps()->originBottomLeft) {
    target_rect.x = display_viewport_.x;
    target_rect.y = display_viewport_.y + display_viewport_.height;
    target_rect.width = display_viewport_.width;
    target_rect.height = -display_viewport_.height;
  }

  screen_quad_->SetPosition(target_rect);
  screen_quad_->SetTexcoord(base::Vec2(screen_buffer_.size));

  encoder->setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
  screen_quad_->Draw(encoder, shader.GetProgram(), render_view);
}

}  // namespace content
