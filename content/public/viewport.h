// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_VIEWPORT_H_
#define CONTENT_PUBLIC_VIEWPORT_H_

#include "base/memory/ref_counted.h"
#include "content/public/disposable.h"
#include "content/public/drawable.h"
#include "content/public/flashable.h"
#include "content/public/graphics.h"
#include "content/public/shader.h"

namespace content {

class Viewport : public base::RefCounted<Viewport>,
                 public GraphicElement,
                 public Disposable,
                 public DrawableParent,
                 public Drawable,
                 public Flashable {
 public:
  Viewport(scoped_refptr<Graphics> screen);
  Viewport(scoped_refptr<Graphics> screen, const base::Rect& rect);
  Viewport(scoped_refptr<Graphics> screen, scoped_refptr<Viewport> viewport);
  ~Viewport() override;

  Viewport(const Viewport&) = delete;
  Viewport& operator=(const Viewport&) = delete;

  void SetOX(int ox);
  int GetOX() {
    CheckIsDisposed();

    return viewport_rect().origin.x;
  }

  void SetOY(int oy);
  int GetOY() {
    CheckIsDisposed();

    return viewport_rect().origin.y;
  }

  scoped_refptr<Color> GetColor() {
    CheckIsDisposed();
    return color_;
  }

  void SetColor(scoped_refptr<Color> color) {
    CheckIsDisposed();
    *color_ = *color;
  }

  scoped_refptr<Tone> GetTone() {
    CheckIsDisposed();
    return tone_;
  }

  void SetTone(scoped_refptr<Tone> tone) {
    CheckIsDisposed();
    *tone_ = *tone;
  }

  void SetRect(scoped_refptr<Rect> rect);
  scoped_refptr<Rect> GetRect() {
    CheckIsDisposed();
    return rect_;
  }

  void SnapToBitmap(scoped_refptr<Bitmap> target);

  void SetViewport(scoped_refptr<Viewport> viewport);
  scoped_refptr<Viewport> GetViewport() {
    CheckDisposed();
    return parent_;
  }

  /* Set custom shader program */
  scoped_refptr<Shader> GetShader() const { return shader_program_; }
  void SetShader(scoped_refptr<Shader> shader);

 private:
  void OnObjectDisposed() override;
  std::string_view DisposedObjectName() const override { return "Viewport"; }

  void BeforeComposite() override;
  void Composite() override;
  void CheckDisposed() const override { CheckIsDisposed(); }
  void OnParentViewportRectChanged(const ViewportInfo& rect) override;

  void InitViewportInternal(const base::Rect& initial_rect);
  void SnapToBitmapInternal(renderer::TextureFrameBuffer* target);
  void OnRectChangedInternal();

  void ApplyViewportEffect(const base::Rect& blend_area,
                           renderer::TextureFrameBuffer& effect_target,
                           const base::Vec4& color,
                           const base::Vec4& tone,
                           scoped_refptr<Shader> program);

  scoped_refptr<Viewport> parent_;

  base::Vec2i parent_offset_;
  scoped_refptr<Rect> rect_;
  scoped_refptr<Color> color_;
  scoped_refptr<Tone> tone_;
  base::CallbackListSubscription rect_observer_;

  scoped_refptr<Shader> shader_program_;
};

class ViewportChild : public Drawable {
 public:
  ViewportChild(scoped_refptr<Graphics> screen,
                scoped_refptr<Viewport> viewport,
                int z = 0,
                int sprite_y = 0);

  ViewportChild(const ViewportChild&) = delete;
  ViewportChild& operator=(const ViewportChild&) = delete;

  virtual void SetViewport(scoped_refptr<Viewport> viewport);
  scoped_refptr<Viewport> GetViewport() {
    CheckDisposed();
    return viewport_;
  }

 private:
  scoped_refptr<Graphics> screen_;
  scoped_refptr<Viewport> viewport_;
};

}  // namespace content

#endif  // !CONTENT_PUBLIC_VIEWPORT_H_