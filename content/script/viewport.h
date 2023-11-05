// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_SCRIPT_VIEWPORT_H_
#define CONTENT_SCRIPT_VIEWPORT_H_

#include "base/memory/ref_counted.h"
#include "content/script/disposable.h"
#include "content/script/drawable.h"
#include "content/script/flashable.h"

namespace content {

class Viewport : public base::RefCounted<Viewport>,
                 public Disposable,
                 public DrawableParent,
                 public Drawable,
                 public Flashable {
 public:
  Viewport();
  Viewport(const base::Rect& rect);
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

    color_ = color;
  }

  scoped_refptr<Tone> GetTone() {
    CheckIsDisposed();

    return tone_;
  }

  void SetTone(scoped_refptr<Tone> tone) {
    CheckIsDisposed();

    tone_ = tone;
  }

 private:
  void OnObjectDisposed() override;
  std::string_view DisposedObjectName() const override { return "Viewport"; }

  void BeforeComposite() override { DrawableParent::NotifyPrepareComposite(); }

  void Composite() override;
  void CheckDisposed() const override { CheckIsDisposed(); }
  void OnViewportRectChanged(const ViewportInfo& rect) override;

  scoped_refptr<Rect> rect_;
  scoped_refptr<Color> color_;
  scoped_refptr<Tone> tone_;
};

class ViewportChild : public Drawable {
 public:
  ViewportChild(scoped_refptr<Viewport> viewport, int z = 0);

  ViewportChild(const ViewportChild&) = delete;
  ViewportChild& operator=(const ViewportChild&) = delete;

  void SetViewport(scoped_refptr<Viewport> viewport);
  scoped_refptr<Viewport> GetViewport() {
    CheckDisposed();

    return viewport_;
  }

 protected:
  virtual void OnViewportChanged() {}

 private:
  scoped_refptr<Viewport> viewport_;
};

}  // namespace content

#endif  // CONTENT_SCRIPT_VIEWPORT_H_