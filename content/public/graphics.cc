// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/graphics.h"

#include "content/config/core_config.h"
#include "content/public/bitmap.h"
#include "content/public/disposable.h"
#include "content/worker/binding_worker.h"
#include "content/worker/event_runner.h"
#include "content/worker/renderer_worker.h"
#include "renderer/quad/quad_drawable.h"

#include "SDL_timer.h"

namespace content {

Graphics::Graphics(scoped_refptr<BindingRunner> dispatcher,
                   scoped_refptr<RenderRunner> renderer,
                   const base::Vec2i& initial_resolution)
    : config_(dispatcher->config()),
      dispatcher_(dispatcher),
      renderer_(renderer),
      resolution_(initial_resolution),
      fps_manager_(std::make_unique<fpslimiter::FPSLimiter>()),
      brightness_(255),
      frozen_(false),
      fps_display_{0, SDL_GetPerformanceCounter()} {
  // Init font attributes
  Font::InitStaticFont();
  default_font_ = new Font();

  viewport_rect().rect = initial_resolution;

  // TODO: rgss1 frame rate
  fps_manager_->SetFrameRate(60);
  frame_rate_ = 60;

  renderer_->PostTask(base::BindOnce(&Graphics::InitScreenBufferInternal,
                                     weak_ptr_factory_.GetWeakPtr()));
}

Graphics::~Graphics() {
  renderer()->PostTask(base::BindOnce(&Graphics::DestroyBufferInternal,
                                      weak_ptr_factory_.GetWeakPtr()));
  renderer()->WaitForSync();
}

int Graphics::GetBrightness() const {
  return brightness_;
}

void Graphics::SetBrightness(int brightness) {
  brightness = std::clamp<int>(brightness, 0, 255);

  if (brightness_ == brightness)
    return;

  brightness_ = brightness;

  renderer()->PostTask(base::BindOnce(&Graphics::SetBrightnessInternal,
                                      weak_ptr_factory_.GetWeakPtr()));
}

void Graphics::Wait(int duration) {
  for (int i = 0; i < duration; ++i) {
    Update();
  }
}

scoped_refptr<Bitmap> Graphics::SnapToBitmap() {
  scoped_refptr<Bitmap> snap = new Bitmap(this, resolution_.x, resolution_.y);

  renderer()->PostTask(base::BindOnce(&Graphics::SnapToBitmapInternal,
                                      weak_ptr_factory_.GetWeakPtr(), snap));

  return snap;
}

void Graphics::FadeOut(int duration) {
  duration = std::max(duration, 1);

  int current_brightness = brightness_;
  for (int i = 0; i < duration; ++i) {
    SetBrightness(current_brightness - current_brightness * (i / duration));
    Update();
  }
}

void Graphics::FadeIn(int duration) {
  duration = std::max(duration, 1);

  int current_brightness = brightness_;
  int diff = 255 - brightness_;
  for (int i = 0; i < duration; ++i) {
    SetBrightness(current_brightness + diff * (i / duration));
    Update();
  }
}

void Graphics::Update() {
  if (frozen_)
    return;

  if (fps_manager_->FrameSkipRequired()) {
    if (config_->allow_frame_skip()) {
      fps_manager_->Delay();
      frame_count_++;

      return;
    } else {
      fps_manager_->ResetFrameSkipCap();
    }
  }

  bool complete_flag = false;
  renderer()->PostTask(base::BindOnce(&Graphics::CompositeScreenInternal,
                                      weak_ptr_factory_.GetWeakPtr()));
  renderer()->PostTask(base::BindOnce(&Graphics::PresentScreenInternal,
                                      weak_ptr_factory_.GetWeakPtr(),
                                      &complete_flag, false));

  /* Delay for desire frame rate */
  fps_manager_->Delay();

  /* If not complete drawing */
  if (!complete_flag) {
    /* Drawtime > expect drawtime, sync for draw complete */
    renderer()->WaitForSync();
  }

  /* Increase frame render count */
  ++frame_count_;

  /* Update fps */
  UpdateAverageFPSInternal();
}

void Graphics::ResizeScreen(const base::Vec2i& resolution) {
  if (resolution_ == resolution)
    return;

  resolution_ = resolution;
  renderer()->PostTask(base::BindOnce(&Graphics::ResizeResolutionInternal,
                                      weak_ptr_factory_.GetWeakPtr()));
  renderer()->WaitForSync();
}

void Graphics::Reset() {
  /* Reset freeze* /
  frozen_ = false;

  /* Reset brightness */
  SetBrightness(255);

  /* Disposed all elements */
  for (auto it = disposable_elements_.tail(); it != disposable_elements_.end();
       it = it->previous()) {
    it->value_as_init()->Dispose();
  }

  /* Reset fpslimiter */
  fps_manager_->ResetFrameSkipCap();
}

void Graphics::Freeze() {
  if (frozen_)
    return;

  renderer()->PostTask(base::BindOnce(&Graphics::FreezeSceneInternal,
                                      weak_ptr_factory_.GetWeakPtr()));
  renderer()->WaitForSync();

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

  renderer()->PostTask(base::BindOnce(&Graphics::TransitionSceneInternal,
                                      weak_ptr_factory_.GetWeakPtr(), duration,
                                      trans_bitmap, vague));

  renderer()->PostTask(
      base::BindOnce([]() { renderer::GSM.states.blend.Push(false); }));

  for (int i = 0; i < duration; ++i) {
    renderer()->PostTask(base::BindOnce(&Graphics::TransitionSceneInternalLoop,
                                        weak_ptr_factory_.GetWeakPtr(), i,
                                        duration, trans_bitmap));

    fps_manager_->Delay();
    frame_count_++;

    /* Break draw loop for quit flag */
    if (dispatcher_->is_quit_required())
      break;
  }

  renderer()->PostTask(
      base::BindOnce([]() { renderer::GSM.states.blend.Pop(); }));
  renderer()->WaitForSync();

  /* Transition process complete */
  frozen_ = false;
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
  fps_manager_->ResetFrameSkipCap();
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

void Graphics::InitScreenBufferInternal() {
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

  brightness_quad_ = std::make_unique<renderer::QuadDrawable>();
  screen_quad_->SetPositionRect(base::Vec2(resolution_));
  screen_quad_->SetTexCoordRect(base::Vec2(resolution_));
  screen_quad_->SetColor();
}

void Graphics::DestroyBufferInternal() {
  renderer::TextureFrameBuffer::Del(screen_buffer_[0]);
  renderer::TextureFrameBuffer::Del(screen_buffer_[1]);
  renderer::TextureFrameBuffer::Del(frozen_snapshot_);

  screen_quad_.reset();
  brightness_quad_.reset();
}

void Graphics::CompositeScreenInternal() {
  if (!brightness_)
    return;

  /* Prepare composite notify */
  DrawableParent::NotifyPrepareComposite();

  renderer::FrameBuffer::Bind(screen_buffer_[0].fbo);
  renderer::GSM.states.clear_color.Set(base::Vec4());
  renderer::FrameBuffer::Clear();

  renderer::GSM.states.scissor_rect.Set(resolution_);
  renderer::GSM.states.viewport.Set(resolution_);

  DrawableParent::CompositeChildren();

  if (brightness_ != 255) {
    auto& shader = renderer::GSM.shaders->color;
    shader.Bind();
    shader.SetProjectionMatrix(resolution_);
    shader.SetTransOffset(base::Vec2());

    brightness_quad_->Draw();
  }

  GLenum errcode = renderer::GL.GetError();
  if (errcode) {
    LOG(INFO) << "[Graphics] GLError: " << errcode;
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

  brightness_quad_->SetPositionRect(base::Vec2(resolution_));
  brightness_quad_->SetTexCoordRect(base::Vec2(resolution_));

  viewport_rect().rect = resolution_;
  NotifyViewportChanged();
}

void Graphics::PresentScreenInternal(bool* paint_raiser, bool backend) {
  // TODO: incorrect content position
  renderer::Blt::BeginScreen(resolution_);
  renderer::Blt::TexSource(screen_buffer_[backend ? 1 : 0]);
  renderer::GSM.states.clear_color.Set(base::Vec4());
  renderer::FrameBuffer::Clear();
  // Flip screen for Y
  base::Rect target_rect(0, resolution_.y, resolution_.x, -resolution_.y);
  renderer::Blt::BltDraw(resolution_, target_rect);
  renderer::Blt::EndDraw();

  SDL_GL_SwapWindow(renderer_->window()->AsSDLWindow());

  if (paint_raiser)
    *paint_raiser = true;
}

void Graphics::SetBrightnessInternal() {
  brightness_quad_->SetColor(-1,
                             base::Vec4(0, 0, 0, (255 - brightness_) / 255.0f));
}

void Graphics::SnapToBitmapInternal(scoped_refptr<Bitmap> target) {
  CompositeScreenInternal();

  renderer::Blt::BeginDraw(target->AsGLType());
  renderer::Blt::TexSource(screen_buffer_[0]);
  renderer::GSM.states.clear_color.Set(base::Vec4());
  renderer::FrameBuffer::Clear();
  renderer::Blt::BltDraw(resolution_, resolution_);
  renderer::Blt::EndDraw();
}

void Graphics::FreezeSceneInternal() {
  CompositeScreenInternal();

  renderer::Blt::BeginDraw(frozen_snapshot_);
  renderer::Blt::TexSource(screen_buffer_[0]);
  renderer::GSM.states.clear_color.Set(base::Vec4());
  renderer::FrameBuffer::Clear();
  renderer::Blt::BltDraw(resolution_, resolution_);
  renderer::Blt::EndDraw();
}

void Graphics::TransitionSceneInternal(int duration,
                                       scoped_refptr<Bitmap> trans_bitmap,
                                       int vague) {
  // Snap to backend buffer
  CompositeScreenInternal();

  auto& alpha_shader = renderer::GSM.shaders->alpha_trans;
  auto& vague_shader = renderer::GSM.shaders->vague_shader;

  if (!trans_bitmap) {
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
  auto& alpha_shader = renderer::GSM.shaders->alpha_trans;
  auto& vague_shader = renderer::GSM.shaders->vague_shader;
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
    vague_shader.SetTransTexture(trans_bitmap->AsGLType().tex);
    vague_shader.SetProgress(progress);
  }

  renderer::FrameBuffer::Bind(screen_buffer_[1].fbo);
  renderer::FrameBuffer::Clear();
  screen_quad_->Draw();

  // present with backend buffer
  PresentScreenInternal(nullptr, true);
}

void Graphics::AddDisposable(Disposable* disp) {
  disposable_elements_.Append(&disp->link_);
}

void Graphics::RemoveDisposable(Disposable* disp) {
  disp->link_.RemoveFromList();
}

void Graphics::RenderEffectRequire(const base::Vec4& color,
                                   const base::Vec4& tone,
                                   const base::Vec4& flash_color) {
  ApplyViewportEffect(screen_buffer_[0], screen_buffer_[1], *screen_quad_,
                      color, tone, flash_color);
}

void Graphics::ApplyViewportEffect(renderer::TextureFrameBuffer& frontend,
                                   renderer::TextureFrameBuffer& backend,
                                   renderer::QuadDrawable& quad,
                                   const base::Vec4& color,
                                   const base::Vec4& tone,
                                   const base::Vec4& flash_color) {
  const base::Rect& viewport_rect = renderer::GSM.states.scissor_rect.Current();
  const base::Rect& screen_rect = resolution_;

  if (tone.w != 0) {
    // Blt front buffer to back
    renderer::GSM.states.scissor.Push(false);
    renderer::Blt::BeginDraw(backend);
    renderer::Blt::TexSource(frontend);
    renderer::Blt::BltDraw(screen_rect, screen_rect);
    renderer::Blt::EndDraw();
    renderer::GSM.states.scissor.Pop();

    renderer::FrameBuffer::Bind(frontend.fbo);
    auto& shader = renderer::GSM.shaders->gray;
    shader.Bind();
    shader.SetProjectionMatrix(renderer::GSM.states.viewport.Current().Size());
    shader.SetGray(tone.w);

    shader.SetTexture(backend.tex);
    shader.SetTextureSize(screen_rect.Size());

    renderer::GSM.states.blend.Push(false);
    quad.Draw();
    renderer::GSM.states.blend.Pop();
  }

  const bool has_tone_effect = (tone.x != 0 || tone.y != 0 || tone.z != 0);
  const bool has_color_effect = color.w != 0;
  const bool has_flash_effect = flash_color.w != 0;

  if (!has_tone_effect && !has_color_effect && !has_flash_effect)
    return;

  auto& shader = renderer::GSM.shaders->flat;
  shader.Bind();
  shader.SetProjectionMatrix(renderer::GSM.states.viewport.Current().Size());

  if (has_tone_effect) {
    base::Vec4 add, sub;

    if (tone.x > 0)
      add.x = tone.x;
    if (tone.y > 0)
      add.y = tone.y;
    if (tone.z > 0)
      add.z = tone.z;

    if (tone.x < 0)
      sub.x = -tone.x;
    if (tone.y < 0)
      sub.y = -tone.y;
    if (tone.z < 0)
      sub.z = -tone.z;

    renderer::GL.BlendFuncSeparate(GL_ONE, GL_ONE, GL_ZERO, GL_ONE);
    if (add.x || add.y || add.z) {
      renderer::GL.BlendEquation(GL_FUNC_ADD);
      shader.SetColor(add);
      quad.Draw();
    }

    if (sub.x || sub.y || sub.z) {
      renderer::GL.BlendEquation(GL_FUNC_REVERSE_SUBTRACT);
      shader.SetColor(sub);
      quad.Draw();
    }
  }

  if (has_color_effect || has_flash_effect) {
    renderer::GL.BlendEquation(GL_FUNC_ADD);
    renderer::GL.BlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
                                   GL_ZERO, GL_ONE);
  }

  if (has_color_effect) {
    shader.SetColor(color);
    quad.Draw();
  }

  if (has_flash_effect) {
    shader.SetColor(flash_color);
    quad.Draw();
  }

  renderer::GSM.states.blend_func.Refresh();
}

void Graphics::UpdateAverageFPSInternal() {
  const uint64_t now_ticks = SDL_GetPerformanceCounter();
  const uint64_t delta_ticks = now_ticks - fps_display_.last_frame_ticks;
  const uint64_t ticks_freq = SDL_GetPerformanceFrequency();

  if (static_cast<double>(delta_ticks) / ticks_freq >= 1.0) {
    average_fps_ = frame_count_ - fps_display_.last_frame_count;

    renderer()->window()->SetTitle(config_->game_title() +
                                   " FPS: " + std::to_string(average_fps_));

    fps_display_.last_frame_count = frame_count_;
    fps_display_.last_frame_ticks = now_ticks;
  }
}

}  // namespace content
