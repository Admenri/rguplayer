// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/graphics.h"

#include "content/public/bitmap.h"
#include "content/public/disposable.h"
#include "content/worker/renderer_worker.h"
#include "renderer/quad/quad_drawable.h"

#include "SDL_timer.h"

namespace content {

Graphics::Graphics(scoped_refptr<RenderRunner> renderer,
                   const base::Vec2i& initial_resolution)
    : renderer_(renderer), resolution_(initial_resolution), brightness_(255) {
  viewport_rect().rect = initial_resolution;

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
  // Ref to RGSS document
  for (int i = 0; i < duration; ++i) {
    Update();
  }
}

scoped_refptr<Bitmap> Graphics::SnapToBitmap() {
  scoped_refptr<Bitmap> snap = new Bitmap(this, resolution_.x, resolution_.y);

  renderer()->PostTask(base::BindOnce(&Graphics::SnapToBitmapInternal,
                                      weak_ptr_factory_.GetWeakPtr(), snap));
  renderer()->WaitForSync();

  return snap;
}

void Graphics::FadeOut(int duration) {}

void Graphics::FadeIn(int duration) {}

void Graphics::Update() {
  // TODO: fps manager required

  bool complete_flag = false;
  renderer()->PostTask(base::BindOnce(&Graphics::PresentScreenInternal,
                                      weak_ptr_factory_.GetWeakPtr(),
                                      &complete_flag));

  /* Delay for desire frame rate */
  SDL_Delay(1000 / 60);

  /* If not complete drawing */
  if (!complete_flag) {
    /* Drawtime > expect drawtime, sync for draw complete */
    renderer()->WaitForSync();
  }

  /* Increase frame render count */
  ++frame_count_;
}

void Graphics::ResizeScreen(const base::Vec2i& resolution) {
  resolution_ = resolution;

  renderer()->PostTask(base::BindOnce(&Graphics::ResizeResolutionInternal,
                                      weak_ptr_factory_.GetWeakPtr()));
  renderer()->WaitForSync();
}

void Graphics::Reset() {
  // Disposed all elements
  for (auto it = disposable_elements_.tail(); it != disposable_elements_.end();
       it = it->previous()) {
    it->value_as_init()->Dispose();
  }

  // TODO: Reset fpslimiter
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
    LOG(ERROR) << "[Graphics] GLError: " << errcode;
  }
}

void Graphics::ResizeResolutionInternal() {
  renderer::TextureFrameBuffer::Alloc(screen_buffer_[0], resolution_.x,
                                      resolution_.y);
  renderer::TextureFrameBuffer::Alloc(screen_buffer_[1], resolution_.x,
                                      resolution_.y);

  screen_quad_->SetPositionRect(base::Vec2(resolution_));
  screen_quad_->SetTexCoordRect(base::Vec2(resolution_));

  brightness_quad_->SetPositionRect(base::Vec2(resolution_));
  brightness_quad_->SetTexCoordRect(base::Vec2(resolution_));

  viewport_rect().rect = resolution_;
  NotifyViewportChanged();
}

void Graphics::PresentScreenInternal(bool* paint_raiser) {
  CompositeScreenInternal();

  renderer::Blt::BeginScreen(resolution_);
  renderer::Blt::TexSource(screen_buffer_[0]);

  renderer::GSM.states.clear_color.Set(base::Vec4());
  renderer::FrameBuffer::Clear();

  base::Rect target_rect(0, resolution_.y, resolution_.x, -resolution_.y);
  renderer::Blt::EndDraw(resolution_, target_rect);

  SDL_GL_SwapWindow(renderer_->window());
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
  renderer::Blt::EndDraw(resolution_, resolution_);
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
  const base::Rect& viewport_rect = renderer::GSM.states.scissor_rect.Current();
  const base::Rect& screen_rect = resolution_;

  if (tone.w != 0) {
    // Blt front buffer to back
    renderer::GSM.states.scissor.Push(false);
    renderer::Blt::BeginDraw(screen_buffer_[1]);
    renderer::Blt::TexSource(screen_buffer_[0]);
    renderer::Blt::EndDraw(screen_rect, screen_rect);
    renderer::GSM.states.scissor.Pop();

    renderer::FrameBuffer::Bind(screen_buffer_[0].fbo);
    auto& shader = renderer::GSM.shaders->gray;
    shader.Bind();
    shader.SetProjectionMatrix(renderer::GSM.states.viewport.Current().Size());
    shader.SetGray(tone.w);

    shader.SetTexture(screen_buffer_[1].tex);
    shader.SetTextureSize(screen_rect.Size());

    renderer::GSM.states.blend.Push(false);
    screen_quad_->Draw();
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
    if (add.x != 0 || add.y != 0 || add.z != 0) {
      renderer::GL.BlendEquation(GL_FUNC_ADD);
      shader.SetColor(add);
      screen_quad_->Draw();
    }

    if (sub.x != 0 || sub.y != 0 || sub.z != 0) {
      renderer::GL.BlendEquation(GL_FUNC_REVERSE_SUBTRACT);
      shader.SetColor(sub);
      screen_quad_->Draw();
    }
  }

  if (has_color_effect || has_flash_effect) {
    renderer::GL.BlendEquation(GL_FUNC_ADD);
    renderer::GL.BlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
                                   GL_ZERO, GL_ONE);
  }

  if (has_color_effect) {
    shader.SetColor(color);
    screen_quad_->Draw();
  }

  if (has_flash_effect) {
    shader.SetColor(flash_color);
    screen_quad_->Draw();
  }

  renderer::GSM.states.blend_func.Refresh();
}

}  // namespace content
