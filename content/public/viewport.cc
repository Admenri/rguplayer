// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/viewport.h"

#include "content/public/bitmap.h"
#include "content/public/utility.h"

namespace content {

Viewport::Viewport(scoped_refptr<Graphics> screen)
    : GraphicsElement(screen.get()),
      Disposable(screen.get()),
      ViewportChild(screen, nullptr) {
  InitViewportInternal(screen->GetSize());
}

Viewport::Viewport(scoped_refptr<Graphics> screen, const base::Rect& rect)
    : GraphicsElement(screen.get()),
      Disposable(screen.get()),
      ViewportChild(screen, nullptr) {
  InitViewportInternal(rect);
}

Viewport::Viewport(scoped_refptr<Graphics> screen,
                   scoped_refptr<Viewport> viewport)
    : GraphicsElement(screen.get()),
      Disposable(screen.get()),
      ViewportChild(screen, viewport) {
  InitViewportInternal(viewport->GetRect()->AsBase());
}

Viewport::~Viewport() {
  Dispose();
}

void Viewport::SetOX(int ox) {
  CheckIsDisposed();

  if (viewport_rect().origin.x == ox)
    return;

  viewport_rect().origin.x = ox;
  NotifyViewportRectChanged();
}

void Viewport::SetOY(int oy) {
  CheckIsDisposed();

  if (viewport_rect().origin.y == oy)
    return;

  viewport_rect().origin.y = oy;
  NotifyViewportRectChanged();
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
}

void Viewport::SetViewport(scoped_refptr<Viewport> viewport) {
  ViewportChild::SetViewport(viewport);

  OnRectChangedInternal();
}

void Viewport::OnObjectDisposed() {
  RemoveFromList();
}

void Viewport::PrepareDraw(bgfx::ViewId* render_view) {
  DrawableParent::PrepareComposite(render_view);
}

void Viewport::OnDraw(CompositeTargetInfo* target_info) {
  if (Flashable::IsFlashing() && Flashable::EmptyFlashing())
    return;

  auto& screen_size = target_info->render_target->size;
  const bool flip_scissor = bgfx::getCaps()->originBottomLeft;
  const int scissor_y = flip_scissor ? (screen_size.y - viewport_rect().rect.y -
                                        viewport_rect().rect.height)
                                     : viewport_rect().rect.y;
  uint16_t scissor_cache = target_info->encoder->setScissor(
      viewport_rect().rect.x, scissor_y, viewport_rect().rect.width,
      viewport_rect().rect.height);

  CompositeTargetInfo::ScissorRegion swap_scissor;
  std::swap(target_info->render_scissor, swap_scissor);

  target_info->render_scissor.cache = scissor_cache;
  target_info->render_scissor.enable = true;
  target_info->render_scissor.region = viewport_rect().rect;

  DrawableParent::Composite(target_info);

  if (Flashable::IsFlashing() || color_->IsValid() || tone_->IsValid()) {
    base::Vec4 composite_color = color_->AsBase();
    base::Vec4 flash_color = Flashable::GetFlashColor();
    base::Vec4 target_color;
    if (Flashable::IsFlashing())
      target_color =
          (flash_color.w > composite_color.w ? flash_color : composite_color);
    else
      target_color = composite_color;

    ApplyViewportEffect(target_info, viewport_rect().rect, target_color,
                        tone_->AsBase());
  }

  std::swap(target_info->render_scissor, swap_scissor);
}

void Viewport::AfterDraw(CompositeTargetInfo* target_info) {
  if (Flashable::IsFlashing() && Flashable::EmptyFlashing())
    return;

  if (Flashable::IsFlashing() || color_->IsValid() || tone_->IsValid()) {
    base::Vec4 composite_color = color_->AsBase();
    base::Vec4 flash_color = Flashable::GetFlashColor();
    base::Vec4 target_color;
    if (Flashable::IsFlashing())
      target_color =
          (flash_color.w > composite_color.w ? flash_color : composite_color);
    else
      target_color = composite_color;

    // Next render pass
    screen()->device()->BindRenderView(++target_info->render_view,
                                       target_info->render_target->size,
                                       target_info->render_target->handle);

    // Composite viewport effect
    ApplyViewportEffect(target_info, viewport_rect().rect, target_color,
                        tone_->AsBase());
  }
}

void Viewport::OnParentViewportRectChanged(const ViewportInfo& rect) {
  OnRectChangedInternal();
}

void Viewport::InitViewportInternal(const base::Rect& initial_rect) {
  rect_ = new Rect(initial_rect);
  rect_observer_ = rect_->AddChangedObserver(base::BindRepeating(
      &Viewport::OnRectChangedInternal, base::Unretained(this)));

  color_ = new Color();
  tone_ = new Tone();

  OnRectChangedInternal();
}

void Viewport::OnRectChangedInternal() {
  viewport_rect().rect = rect_->AsBase();

  base::Vec2i offset = parent_rect().GetRealOffset();
  viewport_rect().rect.x += offset.x;
  viewport_rect().rect.y += offset.y;

  NotifyViewportRectChanged();
}

void Viewport::ApplyViewportEffect(CompositeTargetInfo* target_info,
                                   const base::Rect& blend_area,
                                   const base::Vec4& color,
                                   const base::Vec4& tone) {
  renderer::Texture intermediate_texture;
  screen()->device()->GetGenericTexture(blend_area.Size(),
                                        &intermediate_texture);

  const bool has_tone_effect =
      (tone.x != 0 || tone.y != 0 || tone.z != 0 || tone.w != 0);
  const bool has_color_effect = color.w != 0;

  if (!has_tone_effect && !has_color_effect)
    return;

  bgfx::Encoder* encoder = target_info->encoder;
  bgfx::ViewId render_view = target_info->render_view;

  encoder->blit(render_view, intermediate_texture.handle, 0, 0,
                bgfx::getTexture(target_info->render_target->handle),
                blend_area.x, blend_area.y, blend_area.width,
                blend_area.height);

  auto& shader = screen()->device()->pipelines().viewport;
  auto* quad = screen()->device()->common_quad();

  base::Vec4 offset_size =
      base::MakeVec4(base::Vec2(), base::MakeInvert(intermediate_texture.size));
  encoder->setUniform(shader.OffsetTexSize(), &offset_size);
  encoder->setTexture(0, shader.Texture(), intermediate_texture.handle);
  encoder->setUniform(shader.Color(), &color);
  encoder->setUniform(shader.Tone(), &tone);

  if (target_info->render_scissor.enable)
    encoder->setScissor(target_info->render_scissor.cache);
  encoder->setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);

  quad->SetPosition(blend_area);
  quad->SetTexcoord(base::Rect(blend_area.Size()));
  quad->Draw(encoder, shader.GetProgram(), render_view);
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
  CheckObjectDisposed();

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