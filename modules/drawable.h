// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MODULES_DRAWABLE_H_
#define MODULES_DRAWABLE_H_

#include "base/container/link_list.h"
#include "base/math/math.h"

namespace modules {

class Drawable;

class DrawableManager {
 public:
  struct DrawableViewport {
    base::Rect rect_;
    base::Vec2i origin_;

    base::Vec2i GetRealOffset() const { return rect_.Position() - origin_; }
  };

  DrawableManager();
  virtual ~DrawableManager();

  DrawableManager(const DrawableManager&) = delete;
  DrawableManager& operator=(const DrawableManager) = delete;

  // RenderThread calling
  void Composite();

  // Anythread calling
  void NotifyViewportChanged();

 protected:
  friend class Drawable;

  void Insert(Drawable& element);
  void InsertAfter(Drawable& element, Drawable& after);
  void Reinsert(Drawable& element);

  DrawableViewport drawable_viewport_;
  base::LinkList<Drawable> drawable_children_;
};

class Drawable {
 public:
  Drawable(DrawableManager* frame, int z = 0, bool visible = true);
  virtual ~Drawable();

  Drawable(const Drawable&) = delete;
  Drawable& operator=(const Drawable) = delete;

  void SetDrawableManager(DrawableManager* frame);

  void SetZ(int z);
  int GetZ() const;

  bool IsVisible() const;
  void SetVisible(bool visible);

 protected:
  friend class DrawableManager;

  virtual void Paint() = 0;
  virtual void ViewportRectChanged(
      const DrawableManager::DrawableViewport& viewport) = 0;
  virtual void NeedCheckAccess() const = 0;

  void UnlinkNode();

  bool operator<(const Drawable& other) const;
  int z_ = 0;
  bool visible_ = true;

  DrawableManager* drawable_manager_;
  base::LinkNode<Drawable> node_;
};

}  // namespace modules

#endif  // MODULES_DRAWABLE_H_