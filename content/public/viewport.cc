// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/viewport.h"

#include "content/public/bitmap.h"
#include "content/public/utility.h"
#include "renderer/quad/quad_drawable.h"

namespace content {

Viewport::Viewport(scoped_refptr<Graphics> screen)
    : GraphicElement(screen),
      Disposable(screen.get()),
      Drawable(screen.get(), 0, true) {
  parent_offset_ = screen->viewport_rect().GetRealOffset();
  viewport_rect() = screen->viewport_rect();
  InitViewportInternal(screen->GetSize());
}

Viewport::Viewport(scoped_refptr<Graphics> screen, const base::Rect& rect)
    : GraphicElement(screen),
      Disposable(screen.get()),
      Drawable(screen.get(), 0, true) {
  parent_offset_ = screen->viewport_rect().GetRealOffset();
  viewport_rect().rect = rect;
  viewport_rect().rect.x += parent_offset_.x;
  viewport_rect().rect.y += parent_offset_.y;
  InitViewportInternal(rect);
}

Viewport::Viewport(scoped_refptr<Graphics> screen,
                   scoped_refptr<Viewport> viewport)
    : GraphicElement(screen),
      Disposable(screen.get()),
      Drawable(viewport.get(), 0, true) {
  auto parent_rect = viewport->viewport_rect();
  parent_offset_ = parent_rect.GetRealOffset();
  viewport_rect().rect = base::Rect(parent_offset_, parent_rect.rect.Size());
  InitViewportInternal(parent_rect.rect.Size());
}

Viewport::~Viewport() {
  Dispose();
}

void Viewport::SetOX(int ox) {
  CheckIsDisposed();

  if (viewport_rect().origin.x == ox)
    return;

  viewport_rect().origin.x = ox;
  NotifyViewportChanged();
}

void Viewport::SetOY(int oy) {
  CheckIsDisposed();

  if (viewport_rect().origin.y == oy)
    return;

  viewport_rect().origin.y = oy;
  NotifyViewportChanged();
}

void Viewport::SetRect(scoped_refptr<Rect> rect) {
  CheckIsDisposed();

  if (rect->IsSame(*rect_))
    return;

  *rect_ = *rect;
  OnRectChangedInternal();
}

void Viewport::SnapToBitmap(scoped_refptr<Bitmap> target) {
  CheckIsDisposed();

  screen()->renderer()->PostTask(base::BindOnce(&Viewport::SnapToBitmapInternal,
                                                base::Unretained(this),
                                                target->GetRaw()));
  screen()->renderer()->WaitForSync();
}

void Viewport::SetViewport(scoped_refptr<Viewport> viewport) {
  CheckIsDisposed();

  if (viewport == parent_)
    return;
  parent_ = viewport;

  DrawableParent* parent = parent_.get();
  if (!parent)
    parent = screen().get();
  Drawable::SetParent(parent);
  parent_offset_ = parent->viewport_rect().GetRealOffset();
  OnRectChangedInternal();
}

void Viewport::SetShader(scoped_refptr<Shader> shader) {
  CheckIsDisposed();

  if (shader_program_ == shader)
    return;
  shader_program_ = shader;
}

void Viewport::OnObjectDisposed() {
  RemoveFromList();
}

void Viewport::BeforeComposite() {
  DrawableParent::NotifyPrepareComposite();
}

void Viewport::Composite() {
  if (!shader_program_ && Flashable::IsFlashing() && Flashable::EmptyFlashing())
    return;

  renderer::GSM.states.scissor.Push(true);
  renderer::GSM.states.scissor_rect.PushOnly();
  renderer::GSM.states.scissor_rect.SetIntersect(viewport_rect().rect);

  DrawableParent::CompositeChildren();
  if (Flashable::IsFlashing() || color_->IsValid() || tone_->IsValid() ||
      IsObjectValid(shader_program_.get())) {
    base::Vec4 composite_color = color_->AsBase();
    base::Vec4 flash_color = Flashable::GetFlashColor();
    base::Vec4 target_color;
    if (Flashable::IsFlashing())
      target_color =
          (flash_color.w > composite_color.w ? flash_color : composite_color);
    else
      target_color = composite_color;

    ApplyViewportEffect(viewport_rect().rect, *screen()->GetScreenBuffer(),
                        target_color, tone_->AsBase(), shader_program_);
  }

  renderer::GSM.states.scissor_rect.Pop();
  renderer::GSM.states.scissor.Pop();
}

void Viewport::OnParentViewportRectChanged(const ViewportInfo& rect) {
  parent_offset_ = rect.GetRealOffset();
  OnRectChangedInternal();
}

void Viewport::InitViewportInternal(const base::Rect& initial_rect) {
  rect_ = new Rect(initial_rect);
  rect_observer_ = rect_->AddChangedObserver(base::BindRepeating(
      &Viewport::OnRectChangedInternal, base::Unretained(this)));

  color_ = new Color();
  tone_ = new Tone();
}

void Viewport::OnRectChangedInternal() {
  viewport_rect().rect = rect_->AsBase();
  viewport_rect().rect.x += parent_offset_.x;
  viewport_rect().rect.y += parent_offset_.y;

  NotifyViewportChanged();
}

void Viewport::SnapToBitmapInternal(renderer::TextureFrameBuffer* target) {
  NotifyPrepareComposite();

  renderer::FrameBuffer::Bind(target->fbo);
  renderer::GSM.states.clear_color.Set(base::Vec4());
  renderer::FrameBuffer::Clear();

  if (Flashable::IsFlashing() && Flashable::EmptyFlashing())
    return;

  renderer::GSM.states.scissor.Push(true);
  renderer::GSM.states.scissor_rect.Push(viewport_rect().rect);

  CompositeChildren();
  if (Flashable::IsFlashing() || color_->IsValid() || tone_->IsValid() ||
      IsObjectValid(shader_program_.get())) {
    base::Vec4 composite_color = color_->AsBase();
    base::Vec4 flash_color = Flashable::GetFlashColor();
    base::Vec4 target_color;
    if (Flashable::IsFlashing())
      target_color =
          (flash_color.w > composite_color.w ? flash_color : composite_color);
    else
      target_color = composite_color;

    ApplyViewportEffect(viewport_rect().rect, *target, target_color,
                        tone_->AsBase(), shader_program_);
  }

  renderer::GSM.states.scissor_rect.Pop();
  renderer::GSM.states.scissor.Pop();
}

void Viewport::ApplyViewportEffect(const base::Rect& blend_area,
                                   renderer::TextureFrameBuffer& effect_target,
                                   const base::Vec4& color,
                                   const base::Vec4& tone,
                                   scoped_refptr<Shader> program) {
  auto& temp_fbo =
      renderer::GSM.ClampExplicitTFB(blend_area.width, blend_area.height);

  const bool has_tone_effect =
      (tone.x != 0 || tone.y != 0 || tone.z != 0 || tone.w != 0);
  const bool has_color_effect = color.w != 0;

  if (!program && !has_tone_effect && !has_color_effect)
    return;

  renderer::GSM.states.scissor.Push(false);
  renderer::Blt::BeginDraw(temp_fbo);
  renderer::Blt::TexSource(effect_target);
  renderer::Blt::BltDraw(blend_area, blend_area.Size());

  renderer::FrameBuffer::Bind(effect_target.fbo);
  if (IsObjectValid(program.get())) {
    program->BindShader();
    program->SetInternalUniform();

    GLint texture_location = program->GetUniformLocation("u_texture");
    renderer::GLES2ShaderBase::SetTexture(texture_location, temp_fbo.tex.gl, 0);

    GLint texture_size_location = program->GetUniformLocation("u_texSize");
    renderer::GL.Uniform2f(texture_size_location, 1.0f / blend_area.width,
                           1.0f / blend_area.height);

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
    shader.SetTexture(temp_fbo.tex);
    shader.SetTextureSize(temp_fbo.size);
  }

  renderer::GSM.states.blend.Push(false);
  auto* quad = renderer::GSM.common_quad();
  quad->SetPositionRect(blend_area);
  quad->SetTexCoordRect(base::Rect(blend_area.Size()));
  quad->Draw();
  renderer::GSM.states.blend.Pop();
  renderer::GSM.states.scissor.Pop();
}

ViewportChild::ViewportChild(scoped_refptr<Graphics> screen,
                             scoped_refptr<Viewport> viewport,
                             int z,
                             int sprite_y)
    : Drawable(viewport ? static_cast<DrawableParent*>(viewport.get())
                        : screen.get(),
               z,
               true,
               sprite_y),
      screen_(screen) {}

void ViewportChild::SetViewport(scoped_refptr<Viewport> viewport) {
  CheckDisposed();

  if (viewport == viewport_)
    return;
  viewport_ = viewport;

  DrawableParent* parent = viewport_.get();
  if (!parent)
    parent = screen_.get();
  SetParent(parent);
  OnParentViewportRectChanged(parent->viewport_rect());
}

}  // namespace content