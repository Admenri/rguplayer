// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/graphics.h"

#include "components/fpslimiter/fpslimiter.h"
#include "content/config/core_config.h"
#include "content/public/bitmap.h"
#include "content/public/disposable.h"
#include "content/public/input.h"
#include "content/public/shader.h"
#include "content/worker/binding_worker.h"
#include "content/worker/event_runner.h"
#include "content/worker/renderer_worker.h"
#include "renderer/quad/quad_drawable.h"
#include "third_party/imgui/imgui.h"
#include "third_party/imgui/imgui_impl_opengl3.h"
#include "third_party/imgui/imgui_impl_sdl3.h"

#include "SDL_timer.h"

namespace {

void DrawEngineInfoGUI() {
  if (ImGui::CollapsingHeader("About")) {
    ImGui::Text("Ruby Game Universal (RGU) Runtime");
    ImGui::Separator();
    ImGui::Text("Copyright (C) 2015-2024 Admenri.");
    ImGui::Text("The RGU is licensed under the BSD-3-Clause License, ");
    ImGui::Text("see LICENSE for more information.");

    if (ImGui::Button("Github"))
      SDL_OpenURL("https://github.com/Admenri/rguplayer");
    ImGui::SameLine();
    if (ImGui::Button("AFDian"))
      SDL_OpenURL("https://afdian.net/a/rguplayer");
  }
}

}  // namespace

namespace content {

Graphics::Graphics(WorkerShareData* share_data,
                   base::WeakPtr<BindingRunner> dispatcher,
                   scoped_refptr<RenderRunner> renderer,
                   const base::Vec2i& initial_resolution)
    : share_data_(share_data),
      config_(dispatcher->config()),
      dispatcher_(dispatcher),
      renderer_(renderer),
      resolution_(initial_resolution),
      frozen_(false),
      brightness_(255),
      frame_count_(0),
      frame_rate_(dispatcher->rgss_version() >= RGSSVersion::RGSS2 ? 60 : 40),
      fps_manager_(std::make_unique<fpslimiter::FPSLimiter>(frame_rate_)) {
  viewport_rect().rect = initial_resolution;
  viewport_rect().scissor = false;
  Font::InitStaticFont(dispatcher_->share_data()->filesystem.get());
  InitScreenBufferInternal();

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.IniFilename = nullptr;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

  // Apply DPI Settings
  int display_w, display_h;
  SDL_GetWindowSizeInPixels(window()->AsSDLWindow(), &display_w, &display_h);
  io.DisplaySize = ImVec2((float)display_w, (float)display_h);
  io.DisplayFramebufferScale = ImVec2(1.0, 1.0);
  float windowScale = SDL_GetWindowDisplayScale(window()->AsSDLWindow());
  ImGui::GetStyle().ScaleAllSizes(windowScale);

  // Apply default font
  ImFontConfig font_config;
  font_config.FontDataOwnedByAtlas = false;
  int64_t font_data_size;
  void* font_data = Font::GetDefaultFont(&font_data_size);
  io.Fonts->AddFontFromMemoryTTF(font_data, font_data_size, 16.0f * windowScale,
                                 &font_config,
                                 io.Fonts->GetGlyphRangesChineseFull());

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  // Setup Platform/Renderer backends
  ImGui_ImplSDL3_InitForOpenGL(window()->AsSDLWindow(), renderer_->context());
  ImGui_ImplOpenGL3_Init(nullptr);
}

Graphics::~Graphics() {
  Font::DestroyStaticFont();
  DestroyBufferInternal();

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();
}

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
  SnapToBitmapInternal(snap);
  return snap;
}

void Graphics::FadeOut(int duration) {
  duration = std::max(duration, 1);

  float current_brightness = static_cast<float>(brightness_);
  for (int i = 0; i < duration; ++i) {
    SetBrightness(current_brightness -
                  current_brightness * (i / static_cast<float>(duration)));
    if (frozen_) {
      PresentScreenInternal(frozen_snapshot_);

      FrameProcessInternal();
      if (dispatcher_->CheckRunnerFlags())
        break;
    } else {
      Update();
    }
  }

  /* Set final brightness */
  SetBrightness(0);

  /* Raise flags */
  dispatcher_->RaiseRunnerFlags();
}

void Graphics::FadeIn(int duration) {
  duration = std::max(duration, 1);

  float current_brightness = static_cast<float>(brightness_);
  float diff = 255.0f - current_brightness;
  for (int i = 0; i < duration; ++i) {
    SetBrightness(current_brightness +
                  diff * (i / static_cast<float>(duration)));

    if (frozen_) {
      PresentScreenInternal(frozen_snapshot_);

      FrameProcessInternal();
      if (dispatcher_->CheckRunnerFlags())
        break;
    } else {
      Update();
    }
  }

  /* Set final brightness */
  SetBrightness(255);

  /* Raise flags */
  dispatcher_->RaiseRunnerFlags();
}

void Graphics::Update() {
  if (!frozen_) {
    if (fps_manager_->RequireFrameSkip()) {
      if (config_->allow_frame_skip())
        // Skip render frame
        return FrameProcessInternal();
      else {
        // Reset frame interval diff
        fps_manager_->Reset();
      }
    }

    // Launch render processing
    CompositeScreenInternal();
    PresentScreenInternal(screen_buffer_[0]);
  }

  FrameProcessInternal();

  /* Check flags */
  dispatcher_->CheckRunnerFlags();
  dispatcher_->RaiseRunnerFlags();
}

void Graphics::ResizeScreen(const base::Vec2i& resolution) {
  if (resolution_ == resolution)
    return;

  resolution_ = resolution;
  ResizeResolutionInternal();
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
  SetFrameRate(dispatcher_->rgss_version() >= RGSSVersion::RGSS2 ? 60 : 40);
  SetBrightness(255);
  FrameReset();
}

void Graphics::Freeze() {
  if (frozen_)
    return;
  FreezeSceneInternal();
  frozen_ = true;
}

void Graphics::Transition(int duration,
                          scoped_refptr<Bitmap> trans_bitmap,
                          int vague) {
  if (trans_bitmap && trans_bitmap->IsDisposed())
    return;

  if (!frozen_)
    return;

  SetBrightness(255);
  vague = std::clamp<int>(vague, 1, 256);

  TransitionSceneInternal(duration, !!trans_bitmap, vague);
  renderer::GSM.states.blend.Push(false);
  for (int i = 0; i < duration; ++i) {
    TransitionSceneInternalLoop(i, duration, trans_bitmap);
    FrameProcessInternal();

    /* Break draw loop for quit flag */
    if (dispatcher_->CheckRunnerFlags())
      break;
  }
  renderer::GSM.states.blend.Pop();

  /* Transition process complete */
  frozen_ = false;

  /* Raise signal notify */
  dispatcher_->RaiseRunnerFlags();
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
  window_handle = (uint64_t)SDL_GetProperty(
      SDL_GetWindowProperties(renderer()->window()->AsSDLWindow()),
      "SDL.window.win32.hwnd", NULL);
#else
  // TODO: other platform window handle
#endif
  return window_handle;
}

void Graphics::ResizeWindow(int width, int height) {
  auto* win = renderer()->window()->AsSDLWindow();

  SDL_SetWindowSize(win, width, height);
  SDL_SetWindowPosition(win, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

bool Graphics::GetFullscreen() {
  auto win = renderer()->window();
  return win->IsFullscreen();
}

void Graphics::SetFullscreen(bool fullscreen) {
  auto win = renderer()->window();
  win->SetFullscreen(fullscreen);
}

void Graphics::SetVSync(int interval) {
  vsync_interval_ = interval;
  SetSwapIntervalInternal();
}

int Graphics::GetVSync() {
  return vsync_interval_;
}

bool Graphics::GetFrameSkip() {
  return config_->allow_frame_skip();
}

void Graphics::SetFrameSkip(bool skip) {
  config_->allow_frame_skip() = skip;
}

int Graphics::GetDisplayWidth() {
  return renderer()->window()->GetSize().x;
}

int Graphics::GetDisplayHeight() {
  return renderer()->window()->GetSize().y;
}

void Graphics::SetDrawableOffset(const base::Vec2i& offset) {
  viewport_rect().rect.x = offset.x;
  viewport_rect().rect.y = offset.y;
  NotifyViewportChanged();
}

RGSSVersion Graphics::content_version() const {
  return dispatcher_->rgss_version();
}

filesystem::Filesystem* Graphics::filesystem() {
  return dispatcher_->share_data()->filesystem.get();
}

void Graphics::InitScreenBufferInternal() {
  SDL_GL_GetSwapInterval(&vsync_interval_);

  screen_buffer_[0] = renderer::TextureFrameBuffer::Gen();
  renderer::TextureFrameBuffer::Alloc(screen_buffer_[0], resolution_.x,
                                      resolution_.y);
  renderer::TextureFrameBuffer::LinkFrameBuffer(screen_buffer_[0]);

  screen_buffer_[1] = renderer::TextureFrameBuffer::Gen();
  renderer::TextureFrameBuffer::Alloc(screen_buffer_[1], resolution_.x,
                                      resolution_.y);
  renderer::TextureFrameBuffer::LinkFrameBuffer(screen_buffer_[1]);

  frozen_snapshot_ = renderer::TextureFrameBuffer::Gen();
  renderer::TextureFrameBuffer::Alloc(frozen_snapshot_, resolution_.x,
                                      resolution_.y);
  renderer::TextureFrameBuffer::LinkFrameBuffer(frozen_snapshot_);

  screen_quad_ = std::make_unique<renderer::QuadDrawable>();
  screen_quad_->SetPositionRect(base::Vec2(resolution_));
  screen_quad_->SetTexCoordRect(base::Vec2(resolution_));
}

void Graphics::DestroyBufferInternal() {
  renderer::TextureFrameBuffer::Del(screen_buffer_[0]);
  renderer::TextureFrameBuffer::Del(screen_buffer_[1]);
  renderer::TextureFrameBuffer::Del(frozen_snapshot_);

  screen_quad_.reset();
}

void Graphics::CompositeScreenInternal() {
  /* Prepare composite notify */
  DrawableParent::NotifyPrepareComposite();

  renderer::FrameBuffer::Bind(screen_buffer_[0].fbo);
  renderer::GSM.states.clear_color.Push(base::Vec4(0, 0, 0, 1));
  renderer::FrameBuffer::Clear();
  renderer::GSM.states.clear_color.Pop();

  renderer::GSM.states.scissor_rect.Set(resolution_);
  renderer::GSM.states.viewport.Set(resolution_);

  /* Composite screen to screen buffer */
  DrawableParent::CompositeChildren();

  if (brightness_ < 255) {
    auto& shader = renderer::GSM.shaders()->flat;
    shader.Bind();
    shader.SetProjectionMatrix(resolution_);
    shader.SetColor(base::Vec4(0, 0, 0, (255 - brightness_) / 255.0f));

    screen_quad_->Draw();
  }
}

void Graphics::ResizeResolutionInternal() {
  renderer::TextureFrameBuffer::Alloc(screen_buffer_[0], resolution_.x,
                                      resolution_.y);
  renderer::TextureFrameBuffer::Alloc(screen_buffer_[1], resolution_.x,
                                      resolution_.y);
  renderer::TextureFrameBuffer::Alloc(frozen_snapshot_, resolution_.x,
                                      resolution_.y);

  screen_quad_->SetPositionRect(base::Vec2(resolution_));
  screen_quad_->SetTexCoordRect(base::Vec2(resolution_));

  viewport_rect().rect.width = resolution_.x;
  viewport_rect().rect.height = resolution_.y;
  NotifyViewportChanged();
}

void Graphics::PresentScreenInternal(
    const renderer::TextureFrameBuffer& screen_buffer) {
  base::WeakPtr<ui::Widget> window = renderer_->window();
  UpdateWindowViewportInternal();

  // Flip screen for Y
  base::Rect target_rect;
  window->GetMouseState().resolution = resolution_;
  if (config_->keep_ratio()) {
    target_rect.x = display_viewport_.x;
    target_rect.y = display_viewport_.y + display_viewport_.height;
    target_rect.width = display_viewport_.width;
    target_rect.height = -display_viewport_.height;

    window->GetMouseState().screen_offset = display_viewport_.Position();
    window->GetMouseState().screen = display_viewport_.Size();
  } else {
    target_rect.x = 0;
    target_rect.y = window_size_.y;
    target_rect.width = window_size_.x;
    target_rect.height = -window_size_.y;

    window->GetMouseState().screen_offset = base::Vec2i();
    window->GetMouseState().screen = window_size_;
  }

  // Blit screen buffer to window buffer
  renderer::Blt::BeginScreen(window_size_);
  renderer::FrameBuffer::ClearColor();
  renderer::FrameBuffer::Clear();
  renderer::Blt::TexSource(screen_buffer);
  renderer::Blt::BltDraw(resolution_, target_rect, config_->smooth_scale());
  renderer::Blt::EndDraw();

  // Draw GUI panel
  DrawGUIInternal();

  CheckSyncPoint();
  SDL_GL_SwapWindow(window->AsSDLWindow());
}

void Graphics::SnapToBitmapInternal(scoped_refptr<Bitmap> target) {
  CompositeScreenInternal();

  renderer::Blt::BeginDraw(target->GetTexture());
  renderer::Blt::TexSource(screen_buffer_[0]);
  renderer::Blt::BltDraw(resolution_, resolution_);
  renderer::Blt::EndDraw();
}

void Graphics::FreezeSceneInternal() {
  CompositeScreenInternal();

  renderer::Blt::BeginDraw(frozen_snapshot_);
  renderer::Blt::TexSource(screen_buffer_[0]);
  renderer::Blt::BltDraw(resolution_, resolution_);
  renderer::Blt::EndDraw();
}

void Graphics::TransitionSceneInternal(int duration,
                                       bool has_trans,
                                       int vague) {
  // Snap to backend buffer
  CompositeScreenInternal();

  auto& alpha_shader = renderer::GSM.shaders()->alpha_trans;
  auto& vague_shader = renderer::GSM.shaders()->vague_shader;

  if (!has_trans) {
    alpha_shader.Bind();
    alpha_shader.SetProjectionMatrix(
        renderer::GSM.states.viewport.Current().Size());
    alpha_shader.SetTransOffset(base::Vec2());
    alpha_shader.SetTextureSize(resolution_);
  } else {
    vague_shader.Bind();
    vague_shader.SetProjectionMatrix(
        renderer::GSM.states.viewport.Current().Size());
    vague_shader.SetTransOffset(base::Vec2());
    vague_shader.SetTextureSize(resolution_);
    vague_shader.SetVague(vague / 256.0f);
  }
}

void Graphics::TransitionSceneInternalLoop(int i,
                                           int duration,
                                           scoped_refptr<Bitmap> trans_bitmap) {
  auto& alpha_shader = renderer::GSM.shaders()->alpha_trans;
  auto& vague_shader = renderer::GSM.shaders()->vague_shader;
  float progress = i * (1.0f / duration);

  if (!trans_bitmap) {
    alpha_shader.Bind();
    alpha_shader.SetFrozenTexture(frozen_snapshot_.tex);
    alpha_shader.SetCurrentTexture(screen_buffer_[0].tex);
    alpha_shader.SetProgress(progress);
  } else {
    vague_shader.Bind();
    vague_shader.SetFrozenTexture(frozen_snapshot_.tex);
    vague_shader.SetCurrentTexture(screen_buffer_[0].tex);
    vague_shader.SetTransTexture(trans_bitmap->GetTexture().tex);
    vague_shader.SetProgress(progress);
  }

  renderer::FrameBuffer::Bind(screen_buffer_[1].fbo);
  renderer::FrameBuffer::Clear();
  screen_quad_->Draw();

  // present with backend buffer
  PresentScreenInternal(screen_buffer_[1]);
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
  disposable_elements_.Append(&disp->link_);
}

void Graphics::RemoveDisposable(Disposable* disp) {
  disp->link_.RemoveFromList();
}

void Graphics::RenderEffectRequire(const base::Vec4& color,
                                   const base::Vec4& tone,
                                   scoped_refptr<Shader> program) {
  ApplyViewportEffect(screen_buffer_[0], screen_buffer_[1], *screen_quad_,
                      color, tone, program);
}

void Graphics::ApplyViewportEffect(renderer::TextureFrameBuffer& frontend,
                                   renderer::TextureFrameBuffer& backend,
                                   renderer::QuadDrawable& quad,
                                   const base::Vec4& color,
                                   const base::Vec4& tone,
                                   scoped_refptr<Shader> program) {
  const base::Rect& viewport_rect = renderer::GSM.states.scissor_rect.Current();
  const base::Rect& screen_rect = resolution_;

  const bool has_tone_effect =
      (tone.x != 0 || tone.y != 0 || tone.z != 0 || tone.w != 0);
  const bool has_color_effect = color.w != 0;

  if (!program && !has_tone_effect && !has_color_effect)
    return;

  renderer::GSM.states.scissor.Push(false);
  renderer::Blt::BeginDraw(backend);
  renderer::Blt::TexSource(frontend);
  renderer::Blt::BltDraw(screen_rect, screen_rect);
  renderer::Blt::EndDraw();
  renderer::GSM.states.scissor.Pop();

  renderer::FrameBuffer::Bind(frontend.fbo);
  if (program && !program->IsDisposed()) {
    program->BindShader();
    program->SetInternalUniform();

    GLint texture_location = program->GetUniformLocation("u_texture");
    renderer::GLES2ShaderBase::SetTexture(texture_location, backend.tex.gl, 1);

    GLint texture_size_location = program->GetUniformLocation("u_texSize");
    renderer::GL.Uniform2f(texture_size_location, 1.0f / screen_rect.width,
                           1.0f / screen_rect.height);

    GLint color_location = program->GetUniformLocation("u_color");
    renderer::GL.Uniform4f(color_location, color.x, color.y, color.z, color.w);

    GLint tone_location = program->GetUniformLocation("u_tone");
    renderer::GL.Uniform4f(tone_location, tone.x, tone.y, tone.z, tone.w);
  } else {
    auto& shader = renderer::GSM.shaders()->viewport;
    shader.Bind();
    shader.SetProjectionMatrix(renderer::GSM.states.viewport.Current().Size());
    shader.SetTone(tone);
    shader.SetColor(color);
    shader.SetTexture(backend.tex);
    shader.SetTextureSize(screen_rect.Size());
  }

  renderer::GSM.states.blend.Push(false);
  quad.Draw();
  renderer::GSM.states.blend.Pop();
}

void Graphics::UpdateAverageFPSInternal() {
  SDL_Event quit_event;
  quit_event.type = dispatcher_->share_data()->user_event_id +
                    EventRunner::UPDATE_FPS_DISPLAY;
  SDL_PushEvent(&quit_event);
}

void Graphics::UpdateWindowViewportInternal() {
  base::WeakPtr<ui::Widget> window = renderer_->window();
  window_size_ = window->GetSize();

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

void Graphics::SetSwapIntervalInternal() {
  if (SDL_GL_SetSwapInterval(vsync_interval_))
    LOG(WARNING) << "[Graphics] " << SDL_GetError();
}

void Graphics::CheckSyncPoint() {
  WorkerShareData* data = dispatcher_->share_data();

  if (!data->background_sync.require.load())
    return;

  SDL_GL_MakeCurrent(window()->AsSDLWindow(), nullptr);
  while (!data->background_sync.signal.load())
    SDL_Delay(10);
  SDL_GL_MakeCurrent(window()->AsSDLWindow(), renderer()->context());

  data->background_sync.require.store(false);
  data->background_sync.signal.store(false);
  fps_manager_->Reset();
}

void Graphics::DrawGUIInternal() {
  if (!share_data_->enable_settings_menu)
    return;

  // Event
  SDL_Event sdl_event = {0};
  while (share_data_->event_queue.try_dequeue(sdl_event)) {
    if (share_data_->disable_gui_key_input &&
        (sdl_event.type == SDL_EVENT_KEY_DOWN ||
         sdl_event.type == SDL_EVENT_KEY_UP))
      continue;

    ImGui_ImplSDL3_ProcessEvent(&sdl_event);
  }

  // New frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();

  // Gen window
  DrawSettingsWindowInternal();

  // IME Process
  ImGui::EndFrame();
  share_data_->event_runner->PostTask(base::BindOnce(
      [](bool enable) { enable ? SDL_StartTextInput() : SDL_StopTextInput(); },
      SDL_TextInputActive()));

  // Render
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Graphics::DrawSettingsWindowInternal() {
  ImGui::SetNextWindowPos(ImVec2(), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(350, 400), ImGuiCond_FirstUseEver);

  if (ImGui::Begin("Settings")) {
    // Button settings
    share_data_->create_button_settings_gui.Run();

    // Graphics settings
    DrawGraphicsSettingsGUI();

    // Audio settings
    share_data_->create_audio_settings_gui.Run();

    // Engine Info
    DrawEngineInfoGUI();

    // End window create
    ImGui::End();
  }
}

void Graphics::DrawGraphicsSettingsGUI() {
  if (ImGui::CollapsingHeader("Graphics")) {
    ImGui::TextWrapped("Renderer: %s", renderer::GL.GetString(GL_RENDERER));
    ImGui::Separator();
    ImGui::TextWrapped("Vendor: %s", renderer::GL.GetString(GL_VENDOR));
    ImGui::Separator();
    ImGui::TextWrapped("Version: %s", renderer::GL.GetString(GL_VERSION));
    ImGui::Separator();
    ImGui::Text("Average FPS: %d", share_data_->display_fps);
    ImGui::Separator();

    // V-Sync
    int gl_vsync;
    SDL_GL_GetSwapInterval(&gl_vsync);
    bool ui_vsync = gl_vsync;
    ImGui::Checkbox("V-Sync", &ui_vsync);
    SDL_GL_SetSwapInterval(ui_vsync ? 1 : 0);

    // Smooth Scale
    ImGui::Checkbox("Smooth Scale", &share_data_->config->smooth_scale());

    // ScreenRatio
    ImGui::Checkbox("Keep Ratio", &share_data_->config->keep_ratio());

    // Skip Frame
    ImGui::Checkbox("Frame Skip", &share_data_->config->allow_frame_skip());
  }
}

}  // namespace content
