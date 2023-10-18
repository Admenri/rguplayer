// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_SCRIPT_DRAWABLE_H_
#define CONTENT_SCRIPT_DRAWABLE_H_

#include "base/containers/linked_list.h"
#include "base/math/math.h"

namespace content {

class Drawable;
class DrawableParent;

class DrawableParent {
 public:
  struct ViewportInfo {
    base::Rect rect;
    base::Vec2i origin;

    base::Vec2i GetRealOffset() const { return rect.Position() - origin; }
  };

  DrawableParent();
  virtual ~DrawableParent();

  DrawableParent(const DrawableParent&) = delete;
  DrawableParent& operator=(const DrawableParent&) = delete;

  void InsertDrawable(Drawable* drawable);

  /* Running in render thread */
  void CompositeChildren();

  /* Notify on binding thread */
  void NotifyViewportChanged();

  ViewportInfo& viewport_rect() { return viewport_rect_; }

 private:
  ViewportInfo viewport_rect_;
  base::LinkedList<Drawable> drawables_;
};

class Drawable : public base::LinkNode<Drawable> {
 public:
  Drawable(DrawableParent* parent, int z = 0, bool visible = true);
  virtual ~Drawable();

  Drawable(const Drawable&) = delete;
  Drawable& operator=(const Drawable&) = delete;

  void SetParent(DrawableParent* parent);

  DrawableParent* GetParent() const {
    CheckDisposed();

    return parent_;
  }

  void SetZ(int z);

  int GetZ() const {
    CheckDisposed();

    return z_;
  }

  void SetVisible(bool visible) {
    CheckDisposed();

    visible_ = visible;
  }

  bool GetVisible() const {
    CheckDisposed();

    return visible_;
  }

 protected:
  virtual void Composite() = 0;
  virtual void CheckDisposed() const = 0;
  virtual void OnViewportRectChanged(
      const DrawableParent::ViewportInfo& rect) = 0;

  DrawableParent::ViewportInfo& GetViewportInfo() {
    return parent_->viewport_rect();
  }

 private:
  friend class DrawableParent;
  DrawableParent* parent_;
  int z_;
  bool visible_;
};

}  // namespace content

#endif  // CONTENT_SCRIPT_DRAWABLE_H_