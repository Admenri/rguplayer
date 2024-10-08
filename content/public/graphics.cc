// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/graphics.h"

#include "components/fpslimiter/fpslimiter.h"
#include "content/public/bitmap.h"
#include "content/public/disposable.h"
#include "content/public/input.h"
#include "fiber/fiber.h"
#include "third_party/imgui/imgui.h"
#include "third_party/imgui/imgui_impl_bgfx.h"
#include "third_party/imgui/imgui_impl_sdl3.h"

#include "SDL3/SDL_timer.h"

namespace content {

Graphics::Graphics(CoroutineContext* cc,
                   base::WeakPtr<ui::Widget> window,
                   std::unique_ptr<ScopedFontData> default_font,
                   const base::Vec2i& initial_resolution,
                   APIVersion api_diff)
    : api_version_(api_diff),
      screen_buffer_(BGFX_INVALID_HANDLE),
      frozen_snapshot_(BGFX_INVALID_HANDLE),
      window_size_(window->GetSize()),
      static_font_manager_(std::move(default_font)),
      frozen_(false),
      brightness_(255),
      frame_count_(0),
      frame_rate_(api_diff >= APIVersion ::RGSS2 ? 60 : 40),
      vsync_(0),
      elapsed_time_(0),
      smooth_delta_time_(1),
      last_count_time_(SDL_GetPerformanceCounter()),
      desired_delta_time_(SDL_GetPerformanceFrequency() / frame_rate_),
      cc_(cc) {
  // Initialize root viewport
  viewport_rect().rect = initial_resolution;
  viewport_rect().has_scissor = false;

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

  // Initialize GUI
  ImGui::CreateContext();
  ImGui_Implbgfx_Init(BGFX_CONFIG_MAX_VIEWS - 1);
  ImGui_ImplSDL3_InitForOther(window->AsSDLWindow());
}

Graphics::~Graphics() {
  ImGui_ImplSDL3_Shutdown();
  ImGui_Implbgfx_Shutdown();
  ImGui::DestroyContext();

  screen_quad_.reset();
  if (bgfx::isValid(screen_buffer_.handle))
    bgfx::destroy(screen_buffer_.handle);
  if (bgfx::isValid(frozen_snapshot_.handle))
    bgfx::destroy(frozen_snapshot_.handle);

  device_.reset();
}

bool Graphics::ExecuteEventMainLoop() {
  // Poll event queue
  SDL_Event queued_event;
  while (SDL_PollEvent(&queued_event)) {
    ImGui_ImplSDL3_ProcessEvent(&queued_event);

    if (queued_event.type == SDL_EVENT_QUIT)
      return false;
  }

  // Determine update repeat time
  const uint64_t now_time = SDL_GetPerformanceCounter();
  const uint64_t delta_time = now_time - last_count_time_;
  last_count_time_ = now_time;

  // Calculate smooth frame rate
  const double delta_rate =
      delta_time / static_cast<double>(desired_delta_time_);
  const int repeat_time = DetermineRepeatNumberInternal(delta_rate);

  for (int i = 0; i < repeat_time; ++i) {
    fiber_switch(cc_->main_loop_fiber);

    ImGui_Implbgfx_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    ImGui::ShowDemoWindow();
    ImGui::Render();
    ImGui_Implbgfx_RenderDrawLists(ImGui::GetDrawData());

    bgfx::frame();
  }

  return true;
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
  scoped_refptr<Bitmap> snap = new Bitmap(this, screen_buffer_.size);

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

void Graphics::FadeOut(int duration) {
  duration = std::max(duration, 1);

  float current_brightness = static_cast<float>(brightness_);
  for (int i = 0; i < duration; ++i) {
    SetBrightness(current_brightness -
                  current_brightness * (i / static_cast<float>(duration)));
    if (frozen_) {
      bgfx::Encoder* encoder = bgfx::begin();
      bgfx::ViewId render_view = 1;
      PresentScreenBufferInternal(&frozen_snapshot_, encoder, render_view);
      bgfx::end(encoder);
      FrameProcessInternal();
    } else {
      Update();
    }
  }

  /* Set final brightness */
  SetBrightness(0);
}

void Graphics::FadeIn(int duration) {
  duration = std::max(duration, 1);

  float current_brightness = static_cast<float>(brightness_);
  float diff = 255.0f - current_brightness;
  for (int i = 0; i < duration; ++i) {
    SetBrightness(current_brightness +
                  diff * (i / static_cast<float>(duration)));

    if (frozen_) {
      bgfx::Encoder* encoder = bgfx::begin();
      bgfx::ViewId render_view = 1;
      PresentScreenBufferInternal(&frozen_snapshot_, encoder, render_view);
      bgfx::end(encoder);
      FrameProcessInternal();
    } else {
      Update();
    }
  }

  /* Set final brightness */
  SetBrightness(255);
}

void Graphics::Update() {
  if (!frozen_) {
    // Setup render frame
    bgfx::ViewId render_view = 1;
    bgfx::Encoder* encoder = bgfx::begin();

    // Composite
    EncodeScreenDrawcallsInternal(encoder, &render_view);

    // Present
    PresentScreenBufferInternal(&screen_buffer_, encoder, render_view);

    // Submit to GPU
    bgfx::end(encoder);
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

  // Capture current frame
  bgfx::ViewId render_view = 1;
  bgfx::Encoder* encoder = bgfx::begin();

  // Composite
  EncodeScreenDrawcallsInternal(encoder, &render_view);

  // Blit to bitmap
  encoder->blit(render_view, bgfx::getTexture(frozen_snapshot_.handle), 0, 0,
                bgfx::getTexture(screen_buffer_.handle));

  // Submit to GPU
  bgfx::end(encoder);
  bgfx::frame();

  frozen_ = true;
}

void Graphics::Transition(int duration,
                          scoped_refptr<Bitmap> trans_bitmap,
                          int vague) {
  if (!frozen_)
    return;

  SetBrightness(255);
  vague = std::clamp<int>(vague, 1, 256);

  // Capture current frame
  {
    bgfx::ViewId render_view = 1;
    bgfx::Encoder* encoder = bgfx::begin();
    EncodeScreenDrawcallsInternal(encoder, &render_view);
    bgfx::end(encoder);
    bgfx::frame();
  }

  const bool is_transmap_valid = IsObjectValid(trans_bitmap.get());
  renderer::Framebuffer tmp_framebuffer;
  device()->EnsureCommonFramebuffer(screen_buffer_.size, &tmp_framebuffer,
                                    true);

  for (int i = 0; i < duration; ++i) {
    // Encode a transition frame
    bgfx::ViewId render_view = 1;
    bgfx::Encoder* encoder = bgfx::begin();
    device()->BindRenderView(render_view, tmp_framebuffer.size,
                             tmp_framebuffer.handle, 0);

    {
      float progress = i * (1.0f / duration);
      bgfx::ProgramHandle program = BGFX_INVALID_HANDLE;

      if (is_transmap_valid) {
        auto& shader = device()->pipelines().vaguetrans;

        encoder->setTexture(0, shader.FrozenTexture(),
                            bgfx::getTexture(frozen_snapshot_.handle));
        encoder->setTexture(1, shader.CurrentTexture(),
                            bgfx::getTexture(screen_buffer_.handle));
        encoder->setTexture(2, shader.TransTexture(),
                            bgfx::getTexture(trans_bitmap->GetHandle()));

        base::Vec4 offset_size =
            base::MakeVec4(base::Vec2(), base::MakeInvert(screen_buffer_.size));
        encoder->setUniform(shader.OffsetTexSize(), &offset_size);

        base::Vec4 progress_vague;
        progress_vague.x = progress;
        progress_vague.y = vague / 256.0f;
        encoder->setUniform(shader.ProgressVague(), &progress_vague);

        program = shader.GetProgram();
      } else {
        auto& shader = device()->pipelines().alphatrans;

        encoder->setTexture(0, shader.FrozenTexture(),
                            bgfx::getTexture(frozen_snapshot_.handle));
        encoder->setTexture(1, shader.CurrentTexture(),
                            bgfx::getTexture(screen_buffer_.handle));

        base::Vec4 offset_size =
            base::MakeVec4(base::Vec2(), base::MakeInvert(screen_buffer_.size));
        encoder->setUniform(shader.OffsetTexSize(), &offset_size);

        base::Vec4 uprogress;
        uprogress.x = progress;
        encoder->setUniform(shader.Progress(), &uprogress);

        program = shader.GetProgram();
      }

      bgfx::TransientVertexBuffer tmp_buffer;
      bgfx::allocTransientVertexBuffer(
          &tmp_buffer, 4, renderer::GeometryVertexLayout::GetLayout());
      renderer::GeometryVertexLayout::SetPosition(
          (renderer::GeometryVertexLayout::Data*)tmp_buffer.data,
          base::Vec2(screen_buffer_.size));
      renderer::GeometryVertexLayout::SetTexcoord(
          (renderer::GeometryVertexLayout::Data*)tmp_buffer.data,
          base::Vec2(screen_buffer_.size));
      encoder->setVertexBuffer(0, &tmp_buffer);
      encoder->setIndexBuffer(device()->quad_indices()->GetBufferHandle(), 0,
                              6);

      encoder->setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
      encoder->submit(render_view, program);

      // Present to screen
      PresentScreenBufferInternal(&tmp_framebuffer, encoder, ++render_view);
    }

    // Submit to GPU
    bgfx::end(encoder);

    // Step into next frame
    FrameProcessInternal();
  }

  // Transition process complete
  frozen_ = false;
}

void Graphics::SetFrameRate(int rate) {
  rate = std::max(rate, 1);
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
  frame_count_ = 0;
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
  auto window_size = device()->window()->GetSize();
  bgfx::reset(window_size.x, window_size.y,
              interval ? BGFX_RESET_VSYNC : BGFX_RESET_NONE);
}

int Graphics::GetVSync() {
  return vsync_;
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
  /* Increase frame render count */
  ++frame_count_;

  /* Switch to primary fiber */
  fiber_switch(cc_->primary_fiber);
}

int Graphics::DetermineRepeatNumberInternal(double delta_rate) {
  smooth_delta_time_ *= 0.8;
  smooth_delta_time_ += std::fmin(delta_rate, 2) * 0.2;

  if (smooth_delta_time_ >= 0.9) {
    elapsed_time_ = 0;
    return std::round(elapsed_time_);
  } else {
    elapsed_time_ += delta_rate;
    if (elapsed_time_ >= 1) {
      elapsed_time_ -= 1;
      return 1;
    }
  }

  return 0;
};

void Graphics::AddDisposable(Disposable* disp) {
  disposable_elements_.Append(disp->disposable_link());
}

void Graphics::RemoveDisposable(Disposable* disp) {
  disp->disposable_link()->RemoveFromList();
}

void Graphics::UpdateWindowViewportInternal() {
  auto window_size = device()->window()->GetSize();

  if (!(window_size == window_size_)) {
    window_size_ = window_size;
    bgfx::reset(window_size.x, window_size.y,
                vsync_ ? BGFX_RESET_VSYNC : BGFX_RESET_NONE,
                bgfx::TextureFormat::RGBA8);
  }

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
  // Execute prepare stage (Cost: N view)
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

  // Execute composite (Cost: Limit to 1 view)
  DrawableParent::Composite(&target_info);

  // After composite for viewport effect (Cost: N view)
  (*render_view)++;
  DrawableParent::AfterComposite(encoder, render_view, &screen_buffer_);

  // Apply screen brightness (Cost: 1 view)
  if (brightness_ < 255) {
    device()->BindRenderView(*render_view, screen_buffer_.size,
                             screen_buffer_.handle, std::nullopt);

    auto& shader = device()->pipelines().color;
    screen_quad_->SetPosition(base::Vec2(screen_buffer_.size));
    screen_quad_->SetColor(base::Vec4(0, 0, 0, 1.0 - brightness_ / 255.0f));

    encoder->setState(
        renderer::MakeColorBlendState(renderer::BlendType::Normal));
    screen_quad_->Draw(encoder, shader.GetProgram(), *render_view);

    // Next render pass
    (*render_view)++;
  }
}

void Graphics::PresentScreenBufferInternal(renderer::Framebuffer* target_buffer,
                                           bgfx::Encoder* encoder,
                                           bgfx::ViewId render_view) {
  UpdateWindowViewportInternal();

  device()->window()->GetMouseState().resolution = screen_buffer_.size;
  device()->window()->GetMouseState().screen_offset =
      display_viewport_.Position();
  device()->window()->GetMouseState().screen = display_viewport_.Size();

  auto window_size = device()->window()->GetSize();
  device()->BindRenderView(render_view, window_size, BGFX_INVALID_HANDLE,
                           0x000000ff);

  auto& shader = device()->pipelines().base;

  base::Vec4 offset_size =
      base::MakeVec4(base::Vec2(), base::MakeInvert(target_buffer->size));
  encoder->setUniform(shader.OffsetTexSize(), &offset_size);
  encoder->setTexture(0, shader.Texture(),
                      bgfx::getTexture(target_buffer->handle));

  base::Rect target_rect = display_viewport_;
  if (bgfx::getCaps()->originBottomLeft) {
    target_rect.y = display_viewport_.y + display_viewport_.height;
    target_rect.height = -display_viewport_.height;
  }

  screen_quad_->SetPosition(target_rect);
  screen_quad_->SetTexcoord(base::Vec2(screen_buffer_.size));

  encoder->setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
  screen_quad_->Draw(encoder, shader.GetProgram(), render_view);
}

}  // namespace content
