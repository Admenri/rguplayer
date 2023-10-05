// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MODULES_DRAWABLE_H_
#define MODULES_DRAWABLE_H_

#include "base/container/link_list.h"
#include "base/math/math.h"

namespace modules {

class Drawable;

class DrawFrame {
 public:
  struct ViewportInfo {
    base::Rect rect_;
    base::Vec2i original_point_;

    base::Vec2i GetRealOffset() const {
      return rect_.Position() - original_point_;
    }
  };

  DrawFrame();
  virtual ~DrawFrame();

  DrawFrame(const DrawFrame&) = delete;
  DrawFrame& operator=(const DrawFrame) = delete;

  void DrawablesPaint();
  void NotifyViewportChanged();

 protected:
  friend class Drawable;

  void Insert(Drawable& element);
  void InsertAfter(Drawable& element, Drawable& after);
  void Reinsert(Drawable& element);

  ViewportInfo viewport_;
  base::LinkList<Drawable> drawable_children_;
};

class Drawable {
 public:
  Drawable(DrawFrame* frame);
  virtual ~Drawable();

  Drawable(const Drawable&) = delete;
  Drawable& operator=(const Drawable) = delete;

  void SetParent(DrawFrame* frame);

  void SetZ(int z);
  int GetZ() const;

  bool IsVisible() const;
  void SetVisible(bool visible);

 protected:
  virtual void Paint() = 0;
  virtual void ViewportChanged(const DrawFrame::ViewportInfo& viewport) = 0;
  virtual void NeedCheckAccess() const = 0;

  void UnlinkNode();

  friend class DrawFrame;
  bool operator<(const Drawable& other) const;
  int z_ = 0;
  bool visible_ = true;

  DrawFrame* parent_;
  base::LinkNode<Drawable> node_;
};

}  // namespace modules

#endif  // MODULES_DRAWABLE_H_