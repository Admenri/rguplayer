// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_DRAWABLE_H_
#define CONTENT_PUBLIC_DRAWABLE_H_

#include "base/containers/linked_list.h"
#include "base/math/math.h"

#include <list>

namespace content {

class Drawable;
class DrawableParent;

class DrawableParent {
 public:
  struct ViewportInfo {
    base::Rect rect;
    base::Vec2i origin;

    // Did viewport container has scissor area?
    bool scissor = true;

    // Calculate display offset
    base::Vec2i GetRealOffset() const { return rect.Position() - origin; }
  };

  DrawableParent();
  virtual ~DrawableParent();

  DrawableParent(const DrawableParent&) = delete;
  DrawableParent& operator=(const DrawableParent&) = delete;

  void InsertDrawable(Drawable* drawable);

  /* Running in render thread */
  void NotifyPrepareComposite();
  void CompositeChildren();

  /* Notify on binding thread */
  void NotifyViewportChanged();
  inline ViewportInfo& viewport_rect() { return viewport_rect_; }

 private:
  friend class Drawable;
  bool CalcDrawableOrder(Drawable* self, Drawable* other);

  ViewportInfo viewport_rect_;
  base::LinkedList<Drawable> drawables_;
  base::LinkedList<Drawable> children_;
};

class Drawable {
 public:
  Drawable(DrawableParent* parent,
           int z = 0,
           bool visible = true,
           int sprite_y = 0);
  virtual ~Drawable();

  Drawable(const Drawable&) = delete;
  Drawable& operator=(const Drawable&) = delete;

  void SetParent(DrawableParent* parent);

  DrawableParent* GetParent() const {
    CheckDisposed();

    return parent_;
  }

  virtual void SetZ(int z);
  int GetZ() const {
    CheckDisposed();
    return z_;
  }

  virtual void SetVisible(bool visible) {
    CheckDisposed();
    visible_ = visible;
  }

  bool GetVisible() const {
    CheckDisposed();
    return visible_;
  }

  inline DrawableParent::ViewportInfo& parent_rect() {
    return parent_->viewport_rect();
  }

 protected:
  virtual void BeforeComposite() {}
  virtual void Composite() = 0;
  virtual void CheckDisposed() const = 0;
  virtual void OnParentViewportRectChanged(
      const DrawableParent::ViewportInfo&) {}

  void RemoveFromList();
  void SetSpriteY(int y);

 private:
  friend class DrawableParent;

  base::LinkNode<Drawable> node_;
  base::LinkNode<Drawable> child_;
  DrawableParent* parent_;
  int z_;
  bool visible_;
  uint64_t creation_stamp_;
  int sprite_y_;
};

}  // namespace content

#endif  // !CONTENT_PUBLIC_DRAWABLE_H_