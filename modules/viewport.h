// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MODULES_VIEWPORT_H_
#define MODULES_VIEWPORT_H_

#include "base/math/math.h"
#include "modules/disposable.h"
#include "modules/drawable.h"
#include "modules/flashable.h"
#include "modules/graphics.h"
#include "modules/utility.h"

namespace modules {

class Viewport : public base::RefCounted<Viewport>,
                 public Disposable,
                 public DrawableManager,
                 public Drawable,
                 public Flashable {
 public:
  Viewport(Graphics* screen);
  Viewport(Graphics* screen, int x, int y, int width, int height);
  Viewport(Graphics* screen, const base::Rect& rect);
  ~Viewport() override;

  Viewport(const Viewport&) = delete;
  Viewport& operator=(const Viewport&) = delete;

  void SetRect(scoped_refptr<Rect> rect);
  scoped_refptr<Rect> GetRect() const;

  void SetOX(int ox);
  int GetOX() const;
  void SetOY(int oy);
  int GetOY() const;

  void SetColor(scoped_refptr<Color> color);
  scoped_refptr<Color> GetColor() const;

  void SetTone(scoped_refptr<Tone> tone);
  scoped_refptr<Tone> GetTone() const;

  void InitRefCountedAttributes();

 protected:
  void OnObjectDisposed() override;
  void Paint() override;
  void ViewportRectChanged(const DrawableViewport& viewport) override;
  void NeedCheckAccess() const override { CheckedForDispose(); }

 private:
  void OnRectChanged();
  Graphics* screen_;

  scoped_refptr<Rect> rect_;
  scoped_refptr<Color> color_;
  scoped_refptr<Tone> tone_;

  base::CallbackListSubscription rect_observer_;

  base::WeakPtrFactory<Viewport> weak_ptr_factory_{this};
};

class ViewportDrawable : public Drawable {
 public:
  ViewportDrawable(Graphics* screen, scoped_refptr<Viewport> viewport = nullptr,
                   int z = 0);

  ViewportDrawable(const ViewportDrawable&) = delete;
  ViewportDrawable& operator=(const ViewportDrawable) = delete;

  void SetViewport(scoped_refptr<Viewport> viewport);
  scoped_refptr<Viewport> GetViewport() const;

 protected:
  virtual void OnViewportChange() {}

  Graphics* GetScreen() { return screen_; }

 private:
  Graphics* screen_;
  scoped_refptr<Viewport> viewport_;
};

}  // namespace modules

#endif  // MODULES_VIEWPORT_H_