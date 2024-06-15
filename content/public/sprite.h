// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_SPRITE_H_
#define CONTENT_PUBLIC_SPRITE_H_

#include "base/math/transform.h"
#include "content/public/bitmap.h"
#include "content/public/disposable.h"
#include "content/public/flashable.h"
#include "content/public/graphics.h"
#include "content/public/utility.h"
#include "content/public/viewport.h"
#include "renderer/quad/quad_array.h"

namespace content {

class Sprite : public base::RefCounted<Sprite>,
               public GraphicElement,
               public Disposable,
               public ViewportChild,
               public Flashable {
 public:
  Sprite(scoped_refptr<Graphics> screen,
         scoped_refptr<Viewport> viewport = nullptr);
  ~Sprite() override;

  Sprite(const Sprite&) = delete;
  Sprite& operator=(const Sprite&) = delete;

  int GetWidth() const {
    CheckIsDisposed();
    return src_rect_->GetWidth();
  }

  int GetHeight() const {
    CheckIsDisposed();
    return src_rect_->GetHeight();
  }

  void SetBitmap(scoped_refptr<Bitmap> bitmap);
  scoped_refptr<Bitmap> GetBitmap() {
    CheckIsDisposed();
    return bitmap_;
  }

  void SetSrcRect(scoped_refptr<Rect> rect);
  scoped_refptr<Rect> GetSrcRect() {
    CheckIsDisposed();
    return src_rect_;
  }

  void SetMirror(bool mirror);
  bool GetMirror() const {
    CheckIsDisposed();
    return mirror_;
  }

  void SetOpacity(int opacity) {
    CheckIsDisposed();

    opacity = std::clamp(opacity, 0, 255);
    opacity_ = opacity;
  }

  int GetOpacity() const {
    CheckIsDisposed();
    return opacity_;
  }

  void SetBlendType(renderer::GLBlendType blend_type) {
    CheckIsDisposed();
    blend_mode_ = blend_type;
  }

  renderer::GLBlendType GetBlendType() const {
    CheckIsDisposed();
    return blend_mode_;
  }

  /* Bush depth & opacity */
  void SetBushDepth(int depth) {
    CheckIsDisposed();
    bush_.depth = depth;
  }

  int GetBushDepth() {
    CheckIsDisposed();
    return bush_.depth;
  }

  void SetBushOpacity(int bushOpacity) {
    CheckIsDisposed();
    bush_.opacity = bushOpacity;
  }

  int GetBushOpacity() {
    CheckIsDisposed();
    return bush_.opacity;
  }

  /* Wave emit */
  void SetWaveAmp(int wave_amp) {
    CheckIsDisposed();
    wave_.amp = wave_amp;
  }

  int GetWaveAmp() {
    CheckIsDisposed();
    return wave_.amp;
  }

  void SetWaveLength(int length) {
    CheckIsDisposed();
    wave_.length = length;
  }

  int GetWaveLength() {
    CheckIsDisposed();
    return wave_.length;
  }

  void SetWaveSpeed(int speed) {
    CheckIsDisposed();
    wave_.speed = speed;
  }

  int GetWaveSpeed() {
    CheckIsDisposed();
    return wave_.speed;
  }

  void SetWavePhase(float phase) {
    CheckIsDisposed();
    wave_.phase = phase;
  }

  float GetWavePhase() {
    CheckIsDisposed();
    return wave_.phase;
  }

  scoped_refptr<Color> GetColor() const {
    CheckIsDisposed();
    return color_;
  }

  void SetColor(scoped_refptr<Color> color) {
    CheckIsDisposed();
    *color_ = *color;
  }

  scoped_refptr<Tone> GetTone() const {
    CheckIsDisposed();
    return tone_;
  }

  void SetTone(scoped_refptr<Tone> tone) {
    CheckIsDisposed();
    *tone_ = *tone;
  }

  int GetX() const {
    CheckIsDisposed();
    return transform_.GetPosition().x;
  }

  void SetX(int v) {
    CheckIsDisposed();
    const base::Vec2& i = transform_.GetPosition();
    if (i.x == v)
      return;
    transform_.SetPosition(base::Vec2((float)v, i.y));
  }

  int GetY() const {
    CheckIsDisposed();
    return transform_.GetPosition().y;
  }

  void SetY(int v) {
    CheckIsDisposed();
    const base::Vec2& i = transform_.GetPosition();
    if (i.y == v)
      return;
    transform_.SetPosition(base::Vec2(i.x, (float)v));
    if (screen()->content_version() >= RGSSVersion::RGSS2)
      Drawable::SetSpriteY(v);
  }

  int GetOX() const {
    CheckIsDisposed();
    return transform_.GetOrigin().x;
  }

  void SetOX(int v) {
    CheckIsDisposed();
    const base::Vec2& i = transform_.GetOrigin();
    if (i.x == v)
      return;
    transform_.SetOrigin(base::Vec2((float)v, i.y));
  }

  int GetOY() const {
    CheckIsDisposed();
    return transform_.GetOrigin().y;
  }

  void SetOY(int v) {
    CheckIsDisposed();
    const base::Vec2& i = transform_.GetOrigin();
    if (i.y == v)
      return;
    transform_.SetOrigin(base::Vec2(i.x, (float)v));
  }

  float GetZoomX() const {
    CheckIsDisposed();
    return transform_.GetScale().x;
  }

  void SetZoomX(float v) {
    CheckIsDisposed();
    const base::Vec2& i = transform_.GetScale();
    if (i.x == v)
      return;
    transform_.SetScale(base::Vec2(v, i.y));
  }

  float GetZoomY() const {
    CheckIsDisposed();
    return transform_.GetScale().y;
  }

  void SetZoomY(float v) {
    CheckIsDisposed();
    const base::Vec2& i = transform_.GetScale();
    if (i.y == v)
      return;
    transform_.SetScale(base::Vec2(i.x, v));
  }

  float GetAngle() const {
    CheckIsDisposed();
    return transform_.GetRotation();
  }

  void SetAngle(float v) {
    CheckIsDisposed();
    if (transform_.GetRotation() == v)
      return;
    transform_.SetRotation(v);
  }

  /* Update wave flash */
  void Update() override;

 private:
  void InitAttributeInternal();

  void OnObjectDisposed() override;
  std::string_view DisposedObjectName() const override { return "Sprite"; }

  void BeforeComposite() override;
  void Composite() override;
  void CheckDisposed() const override { CheckIsDisposed(); }
  void OnParentViewportRectChanged(
      const DrawableParent::ViewportInfo& rect) override;
  void OnSrcRectChangedInternal();
  void UpdateWaveQuadsInternal();
  void UpdateVisibilityInternal();

  scoped_refptr<Bitmap> bitmap_;
  scoped_refptr<Rect> src_rect_;
  base::TransformMatrix transform_;

  struct {
    bool active = false;
    int amp = 0;
    int length = 180;
    int speed = 360;
    float phase = 0.0f;

    bool need_update = true;
  } wave_;

  bool mirror_ = false;

  struct {
    int depth = 0;
    int opacity = 128;
  } bush_;

  int opacity_ = 255;

  renderer::GLBlendType blend_mode_ = renderer::GLBlendType::Normal;

  scoped_refptr<Color> color_;
  scoped_refptr<Tone> tone_;

  renderer::QuadDrawable* quad_;
  renderer::QuadDrawableArray<renderer::CommonVertex>* wave_quads_;

  base::CallbackListSubscription src_rect_observer_;

  bool need_invisible_ = false;
  bool src_rect_need_update_ = false;
};

}  // namespace content

#endif  // !CONTENT_PUBLIC_SPRITE_H_