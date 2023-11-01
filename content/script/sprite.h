// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_SCRIPT_SPRITE_H_
#define CONTENT_SCRIPT_SPRITE_H_

#include "base/math/transform.h"
#include "content/scheduler/worker_cc.h"
#include "content/script/bitmap.h"
#include "content/script/disposable.h"
#include "content/script/flashable.h"
#include "content/script/viewport.h"

namespace content {

class Sprite : public base::RefCounted<Sprite>,
               public Disposable,
               public ViewportChild,
               public Flashable {
 public:
  Sprite(scoped_refptr<Viewport> viewport = nullptr);
  ~Sprite() override;

  Sprite(const Sprite&) = delete;
  Sprite& operator=(const Sprite&) = delete;

  int GetWidth() const { return src_rect_->GetWidth(); }
  int GetHeight() const { return src_rect_->GetHeight(); }

  void SetBitmap(scoped_refptr<Bitmap> bitmap);
  scoped_refptr<Bitmap> GetBitmap() { return bitmap_; }

  void SetSrcRect(scoped_refptr<Rect> rect);
  scoped_refptr<Rect> GetRect() { return src_rect_; }

  void SetMirror(bool mirror);
  bool GetMirror() const { return mirror_; }

  void SetOpacity(int opacity) { opacity_ = opacity; }
  int GetOpacity() const { return opacity_; }

  void SetBlendMode(gpu::GLBlendType blend_type) { blend_mode_ = blend_type; }
  gpu::GLBlendType GetBlendMode() const { return blend_mode_; }

  void Update() override;

  /* Non-threaded safe, only for testing */
  base::TransformMatrix& GetTransform() {
    CheckIsDisposed();

    return transform_;
  }

 private:
  void InitAttributeInternal();
  void InitSpriteInternal();

  void OnObjectDisposed() override;
  std::string_view DisposedObjectName() const override { return "Sprite"; }

  void Composite() override;
  void CheckDisposed() const override { CheckIsDisposed(); }
  void OnViewportRectChanged(const DrawableParent::ViewportInfo& rect) override;

  void OnSrcRectChangedInternal();
  void AsyncSrcRectChangedInternal();
  void OnViewportRectChangedInternal(const DrawableParent::ViewportInfo& rect);

  scoped_refptr<Bitmap> bitmap_;
  scoped_refptr<Rect> src_rect_;
  base::TransformMatrix transform_;

  struct {
    int amp = 0;
    int length = 0;
    int speed = 0;
    int phase = 0;
  } wave_;

  bool mirror_ = false;

  struct {
    int depth = 0;
    int opacity = 0;
  } bush_;

  int opacity_ = 255;

  gpu::GLBlendType blend_mode_ = gpu::GLBlendType::Normal;

  scoped_refptr<Color> color_;
  scoped_refptr<Tone> tone_;

  std::unique_ptr<gpu::QuadDrawable> quad_;

  base::CallbackListSubscription src_rect_observer_;

  base::WeakPtrFactory<Sprite> weak_ptr_factory_{this};
};

}  // namespace content

#endif  // CONTENT_SCRIPT_SPRITE_H_