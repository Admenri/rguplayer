// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/graphics.h"

#include "components/fpslimiter/fpslimiter.h"
#include "content/common/command_ids.h"
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

void DrawEngineInfoGUI(scoped_refptr<content::CoreConfigure> config) {
  if (ImGui::CollapsingHeader(
          config->GetI18NString(IDS_SETTINGS_ABOUT, "About").c_str())) {
    ImGui::Text("Ruby Game Universal (RGU) Runtime");
    ImGui::Separator();
    ImGui::Text("Copyright (C) 2015-2024 Admenri.");
    ImGui::TextWrapped(
        "The RGU is licensed under the BSD-3-Clause License, see LICENSE for "
        "more information.");

    if (ImGui::Button("Github"))
      SDL_OpenURL("https://github.com/Admenri/rguplayer");
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
      fps_manager_(std::make_unique<fpslimiter::FPSLimiter>(frame_rate_)),
      vsync_interval_(0) {
  viewport_rect().rect = initial_resolution;
  viewport_rect().scissor = false;
  static_font_manager_ = std::make_unique<ScopedFontData>(
      config_, dispatcher_->share_data()->filesystem.get());

  renderer_->PostTask(base::BindOnce(&Graphics::InitScreenBufferInternal,
                                     base::Unretained(this)));
  renderer_->WaitForSync();

  fps_manager_->Reset();
}

Graphics::~Graphics() {
  static_font_manager_.reset();

  renderer_->PostTask(
      base::BindOnce(&Graphics::DestroyBufferInternal, base::Unretained(this)));
  renderer_->WaitForSync();
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

  renderer_->PostTask(base::BindOnce(&Graphics::SnapToBitmapInternal,
                                     base::Unretained(this), snap->GetRaw()));
  renderer_->WaitForSync();

  return snap;
}

void Graphics::FadeOut(int duration) {
  duration = std::max(duration, 1);

  float current_brightness = static_cast<float>(brightness_);
  for (int i = 0; i < duration; ++i) {
    SetBrightness(current_brightness -
                  current_brightness * (i / static_cast<float>(duration)));
    if (frozen_) {
      renderer_->PostTask(base::BindOnce(&Graphics::PresentScreenInternal,
                                         base::Unretained(this),
                                         frozen_snapshot_));
      renderer_->WaitForSync();

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
      renderer_->PostTask(base::BindOnce(&Graphics::PresentScreenInternal,
                                         base::Unretained(this),
                                         frozen_snapshot_));
      renderer_->WaitForSync();

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
  std::atomic_bool sync_fence = false;
  if (!frozen_) {
    if (fps_manager_->RequireFrameSkip()) {
      if (config_->allow_frame_skip()) {
        // Skip render frame
        return FrameProcessInternal();
      } else {
        // Reset frame interval diff
        fps_manager_->Reset();
      }
    }

    // Launch render processing
    renderer_->PostTask(base::BindOnce(&Graphics::UpdateInternal,
                                       base::Unretained(this), &sync_fence));
  } else {
    // Invalid sync
    sync_fence = true;
  }

  // Process frame delay
  FrameProcessInternal();

  if (!sync_fence)
    renderer_->WaitForSync();

  /* Check flags */
  dispatcher_->CheckRunnerFlags();
  dispatcher_->RaiseRunnerFlags();
}

void Graphics::ResizeScreen(const base::Vec2i& resolution) {
  if (resolution_ == resolution)
    return;

  resolution_ = resolution;
  renderer_->PostTask(base::BindOnce(&Graphics::ResizeResolutionInternal,
                                     base::Unretained(this)));
  renderer_->WaitForSync();
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
  renderer_->PostTask(
      base::BindOnce(&Graphics::FreezeSceneInternal, base::Unretained(this)));
  renderer_->WaitForSync();
  frozen_ = true;
}

void Graphics::Transition(int duration,
                          scoped_refptr<Bitmap> trans_bitmap,
                          int vague) {
  if (!frozen_)
    return;

  SetBrightness(255);
  vague = std::clamp<int>(vague, 1, 256);
  const bool is_transmap_valid = IsObjectValid(trans_bitmap.get());

  renderer_->PostTask(base::BindOnce(&Graphics::TransitionSceneInternal,
                                     base::Unretained(this), duration,
                                     is_transmap_valid, vague));

  renderer_->PostTask(
      base::BindOnce([]() { renderer::GSM.states.blend.Push(false); }));

  for (int i = 0; i < duration; ++i) {
    renderer_->PostTask(base::BindOnce(
        &Graphics::TransitionSceneInternalLoop, base::Unretained(this), i,
        duration, is_transmap_valid ? trans_bitmap->GetRaw() : nullptr));
    FrameProcessInternal();

    /* Break draw loop for quit flag */
    if (dispatcher_->CheckRunnerFlags())
      break;
  }

  renderer_->PostTask(
      base::BindOnce([]() { renderer::GSM.states.blend.Pop(); }));

  /* Transition process complete */
  frozen_ = false;

  renderer_->WaitForSync();

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
  window_handle = (uint64_t)SDL_GetPointerProperty(
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
  renderer_->PostTask(base::BindOnce(&Graphics::SetSwapIntervalInternal,
                                     base::Unretained(this)));
  renderer_->WaitForSync();
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

bool Graphics::GetBackgroundRunning() {
  return config_->background_running();
}

void Graphics::SetBackgroundRunning(bool allow) {
  config_->background_running() = allow;
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

base::Vec2i Graphics::GetDrawableOffset() {
  return viewport_rect().rect.Position();
}

void Graphics::SetWindowFavicon(scoped_refptr<Bitmap> icon) {
  SDL_SetWindowIcon(window()->AsSDLWindow(), icon->SurfaceRequired());
}

void Graphics::SetWindowTitle(const std::string& title) {
  window()->SetTitle(title);
}

void Graphics::SetWindowMinimumSize(const base::Vec2i& size) {
  SDL_SetWindowMinimumSize(window()->AsSDLWindow(), size.x, size.y);
}

void Graphics::SetWindowAspectRatio(float min_ratio, float max_ratio) {
  SDL_SetWindowAspectRatio(window()->AsSDLWindow(), min_ratio, max_ratio);
}

void Graphics::SetWindowAlwaysOnTop(bool top) {
  SDL_SetWindowAlwaysOnTop(window()->AsSDLWindow(), top);
}

RGSSVersion Graphics::content_version() const {
  return dispatcher_->rgss_version();
}

filesystem::Filesystem* Graphics::filesystem() {
  return dispatcher_->share_data()->filesystem.get();
}

renderer::TextureFrameBuffer* Graphics::AllocTexture(const base::Vec2i& size,
                                                     bool clean,
                                                     GLenum format,
                                                     void* buffer,
                                                     size_t buffer_size) {
  renderer::TextureFrameBuffer* mem = new renderer::TextureFrameBuffer;
  std::vector<uint8_t> texture_data;
  if (buffer_size && buffer) {
    texture_data.assign(buffer_size, 0);
    memcpy(texture_data.data(), buffer, buffer_size);
  }

  renderer()->PostTask(base::BindOnce(
      [](renderer::TextureFrameBuffer* ptr, const base::Vec2i& tex_size,
         bool need_clean, GLenum tex_format, std::vector<uint8_t> data) {
        *ptr = renderer::TextureFrameBuffer::Gen();

        renderer::TextureFrameBuffer::Alloc(*ptr, tex_size, tex_format);
        if (!data.empty())
          renderer::Texture::TexImage2D(tex_size.x, tex_size.y, tex_format,
                                        data.data());

        renderer::TextureFrameBuffer::LinkFrameBuffer(*ptr);
        if (need_clean)
          renderer::FrameBuffer::Clear();
      },
      mem, size, clean, format, std::move(texture_data)));

  return mem;
}

void Graphics::FreeTexture(renderer::TextureFrameBuffer*& texture_data) {
  renderer()->PostTask(base::BindOnce(
      [](renderer::TextureFrameBuffer* texture) {
        renderer::TextureFrameBuffer::Del(*texture);
        delete texture;
      },
      texture_data));
  texture_data = nullptr;
}

void Graphics::InitScreenBufferInternal() {
  screen_buffer_ = renderer::TextureFrameBuffer::Gen();
  renderer::TextureFrameBuffer::Alloc(screen_buffer_, resolution_);
  renderer::TextureFrameBuffer::LinkFrameBuffer(screen_buffer_);

  frozen_snapshot_ = renderer::TextureFrameBuffer::Gen();
  renderer::TextureFrameBuffer::Alloc(frozen_snapshot_, resolution_);
  renderer::TextureFrameBuffer::LinkFrameBuffer(frozen_snapshot_);

  screen_quad_ = std::make_unique<renderer::QuadDrawable>();
  screen_quad_->SetPositionRect(base::Vec2(resolution_));
  screen_quad_->SetTexCoordRect(base::Vec2(resolution_));

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
  int64_t font_data_size;
  void* font_data = static_font_manager_->GetUIDefaultFont(&font_data_size);
  if (font_data) {
    ImFontConfig font_config;
    font_config.FontDataOwnedByAtlas = false;
    io.Fonts->AddFontFromMemoryTTF(font_data, font_data_size,
                                   16.0f * windowScale, &font_config,
                                   io.Fonts->GetGlyphRangesChineseFull());
  } else {
    // No default for GUI
    LOG(INFO) << "[GUI] Invalid default font, use internal font.";
  }

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  // Setup Platform/Renderer backends
  ImGui_ImplSDL3_InitForOpenGL(window()->AsSDLWindow(), renderer_->context());
  ImGui_ImplOpenGL3_Init(nullptr);
}

void Graphics::DestroyBufferInternal() {
  renderer::TextureFrameBuffer::Del(screen_buffer_);
  renderer::TextureFrameBuffer::Del(frozen_snapshot_);

  screen_quad_.reset();

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();
}

void Graphics::CompositeScreenInternal() {
  /* Prepare composite notify */
  DrawableParent::NotifyPrepareComposite();

  renderer::FrameBuffer::Bind(screen_buffer_.fbo);
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
  renderer::TextureFrameBuffer::Alloc(screen_buffer_, resolution_);
  renderer::TextureFrameBuffer::Alloc(frozen_snapshot_, resolution_);

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

  // Draw GUI panel
  DrawGUIInternal();

  GLenum gl_error = renderer::GL.GetError();
  if (gl_error != GL_NO_ERROR)
    LOG(INFO) << "[Graphics] GLError: " << gl_error;

  CheckSyncPoint();
  renderer()->context()->SwapBuffers();
}

void Graphics::SnapToBitmapInternal(renderer::TextureFrameBuffer* target) {
  CompositeScreenInternal();

  renderer::Blt::BeginDraw(*target);
  renderer::Blt::TexSource(screen_buffer_);
  renderer::Blt::BltDraw(resolution_, resolution_);
}

void Graphics::FreezeSceneInternal() {
  CompositeScreenInternal();

  renderer::Blt::BeginDraw(frozen_snapshot_);
  renderer::Blt::TexSource(screen_buffer_);
  renderer::Blt::BltDraw(resolution_, resolution_);
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

void Graphics::TransitionSceneInternalLoop(
    int i,
    int duration,
    renderer::TextureFrameBuffer* trans_bitmap) {
  auto& alpha_shader = renderer::GSM.shaders()->alpha_trans;
  auto& vague_shader = renderer::GSM.shaders()->vague_shader;
  float progress = i * (1.0f / duration);

  if (!trans_bitmap) {
    alpha_shader.Bind();
    alpha_shader.SetFrozenTexture(frozen_snapshot_.tex);
    alpha_shader.SetCurrentTexture(screen_buffer_.tex);
    alpha_shader.SetProgress(progress);
  } else {
    vague_shader.Bind();
    vague_shader.SetFrozenTexture(frozen_snapshot_.tex);
    vague_shader.SetCurrentTexture(screen_buffer_.tex);
    vague_shader.SetTransTexture(trans_bitmap->tex);
    vague_shader.SetProgress(progress);
  }

  auto& temp_fbo = renderer::GSM.EnsureCommonTFB(resolution_.x, resolution_.y);
  renderer::FrameBuffer::Bind(temp_fbo.fbo);
  renderer::FrameBuffer::Clear();
  screen_quad_->Draw();

  // present with backend buffer
  PresentScreenInternal(temp_fbo);
}

void Graphics::FrameProcessInternal() {
  /* Control frame delay */
  fps_manager_->Delay();

  /* Increase frame render count */
  ++frame_count_;

  /* Update average fps */
  UpdateAverageFPSInternal();
}

void Graphics::UpdateInternal(std::atomic_bool* fence) {
  CompositeScreenInternal();
  PresentScreenInternal(screen_buffer_);
  *fence = true;
}

void Graphics::AddDisposable(Disposable* disp) {
  disposable_elements_.Append(&disp->link_);
}

void Graphics::RemoveDisposable(Disposable* disp) {
  disp->link_.RemoveFromList();
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
  renderer()->context()->SetInterval(vsync_interval_);
}

void Graphics::CheckSyncPoint() {
  WorkerShareData* data = dispatcher_->share_data();

  if (!data->background_sync.require.load())
    return;

  renderer()->context()->MakeCurrent(true);
  while (!data->background_sync.signal.load())
    SDL_Delay(10);
  renderer()->context()->MakeCurrent(false);

  data->background_sync.require.store(false);
  data->background_sync.signal.store(false);
  fps_manager_->Reset();
}

void Graphics::DrawGUIInternal() {
  if (!share_data_->enable_settings_menu) {
    share_data_->menu_window_focused = false;
    return;
  }

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
      [](SDL_Window* window, bool enable) {
        enable ? SDL_StartTextInput(window) : SDL_StopTextInput(window);
      },
      window()->AsSDLWindow(), SDL_TextInputActive(window()->AsSDLWindow())));

  // Render
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Graphics::DrawSettingsWindowInternal() {
  ImGui::SetNextWindowPos(ImVec2(), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(350, 400), ImGuiCond_FirstUseEver);

  if (ImGui::Begin(
          config_->GetI18NString(IDS_MENU_SETTINGS, "Settings").c_str())) {
    share_data_->menu_window_focused = ImGui::IsWindowFocused();

    // Button settings
    share_data_->create_button_settings_gui.Run();

    // Graphics settings
    DrawGraphicsSettingsGUI();

    // Audio settings
    share_data_->create_audio_settings_gui.Run();

    // Engine Info
    DrawEngineInfoGUI(config_);

    // End window create
    ImGui::End();
  }
}

void Graphics::DrawGraphicsSettingsGUI() {
  if (ImGui::CollapsingHeader(
          config_->GetI18NString(IDS_SETTINGS_GRAPHICS, "Graphics").c_str())) {
    static std::string ogl_renderer(
        (const char*)renderer::GL.GetString(GL_RENDERER));
    static std::string ogl_vendor(
        (const char*)renderer::GL.GetString(GL_VENDOR));
    static std::string ogl_version(
        (const char*)renderer::GL.GetString(GL_VERSION));

    ImGui::TextWrapped(
        "%s: %s",
        config_->GetI18NString(IDS_GRAPHICS_RENDERER, "Renderer").c_str(),
        ogl_renderer.c_str());
    ImGui::Separator();
    ImGui::TextWrapped(
        "%s: %s", config_->GetI18NString(IDS_GRAPHICS_VENDOR, "Vendor").c_str(),
        ogl_vendor.c_str());
    ImGui::Separator();
    ImGui::TextWrapped(
        "%s: %s",
        config_->GetI18NString(IDS_GRAPHICS_VERSION, "Version").c_str(),
        ogl_version.c_str());
    ImGui::Separator();
    ImGui::Text(
        "%s: %d",
        config_->GetI18NString(IDS_GRAPHICS_AVERAGE_FPS, "Average FPS").c_str(),
        share_data_->display_fps);
    ImGui::Separator();

    // V-Sync
    bool ui_vsync = vsync_interval_;
    ImGui::Checkbox(
        config_->GetI18NString(IDS_GRAPHICS_VSYNC, "V-Sync").c_str(),
        &ui_vsync);
    vsync_interval_ = ui_vsync;
    renderer()->context()->SetInterval(vsync_interval_);

    // Smooth Scale
    ImGui::Checkbox(
        config_->GetI18NString(IDS_GRAPHICS_SMOOTH_SCALE, "Smooth Scale")
            .c_str(),
        &share_data_->config->smooth_scale());

    // ScreenRatio
    ImGui::Checkbox(
        config_->GetI18NString(IDS_GRAPHICS_KEEP_RATIO, "Keep Ratio").c_str(),
        &share_data_->config->keep_ratio());

    // Skip Frame
    ImGui::Checkbox(
        config_->GetI18NString(IDS_GRAPHICS_SKIP_FRAME, "Skip Frame").c_str(),
        &share_data_->config->allow_frame_skip());

    // Fullscreen
    bool is_fullscreen = window()->IsFullscreen(),
         last_fullscreen = is_fullscreen;
    ImGui::Checkbox(
        config_->GetI18NString(IDS_GRAPHICS_FULLSCREEN, "Fullscreen").c_str(),
        &is_fullscreen);
    if (last_fullscreen != is_fullscreen)
      window()->SetFullscreen(is_fullscreen);

    // Background running
    ImGui::Checkbox(config_
                        ->GetI18NString(IDS_GRAPHICS_BACKGROUND_RUNNING,
                                        "Background Running")
                        .c_str(),
                    &share_data_->config->background_running());
  }
}

}  // namespace content
