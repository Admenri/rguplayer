// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "modules/drawable.h"

#include "drawable.h"

namespace modules {

DrawFrame::DrawFrame() {}

DrawFrame::~DrawFrame() {
  for (base::LinkNode<Drawable>* it = drawable_children_.begin();
       it != drawable_children_.end(); it = it->Next()) {
    it->Data()->parent_ = nullptr;
  }
}

void DrawFrame::DrawablesPaint() {
  for (base::LinkNode<Drawable>* it = drawable_children_.begin();
       it != drawable_children_.end(); it = it->Next()) {
    if (it->Data()->IsVisible()) it->Data()->Paint();
  }
}

void DrawFrame::NotifyViewportChanged() {
  for (base::LinkNode<Drawable>* it = drawable_children_.begin();
       it != drawable_children_.end(); it = it->Next()) {
    it->Data()->ViewportChanged(viewport_);
  }
}

void DrawFrame::Insert(Drawable& element) {
  base::LinkNode<Drawable>* iter;

  for (iter = drawable_children_.begin(); iter != drawable_children_.end();
       iter = iter->Next()) {
    Drawable* e = iter->Data();

    if (element < *e) {
      drawable_children_.InsertBefore(element.node_, *iter);
      return;
    }
  }

  drawable_children_.PushBack(element.node_);
}

void DrawFrame::InsertAfter(Drawable& element, Drawable& after) {
  base::LinkNode<Drawable>* iter;

  for (iter = &after.node_; iter != drawable_children_.end();
       iter = iter->Next()) {
    Drawable* e = iter->Data();

    if (element < *e) {
      drawable_children_.InsertBefore(element.node_, *iter);
      return;
    }
  }

  drawable_children_.PushBack(element.node_);
}

void DrawFrame::Reinsert(Drawable& element) {
  drawable_children_.Remove(element.node_);
  Insert(element);
}

Drawable::Drawable(DrawFrame* frame) : parent_(frame), node_(this) {}

Drawable::~Drawable() { UnlinkNode(); }

void Drawable::SetParent(DrawFrame* frame) {
  NeedCheckAccess();

  UnlinkNode();

  parent_ = frame;
  frame->Insert(*this);
  ViewportChanged(frame->viewport_);
}

void Drawable::SetZ(int z) {
  NeedCheckAccess();

  if (z_ == z) return;

  z_ = z;

  parent_->Reinsert(*this);
}

int Drawable::GetZ() const {
  NeedCheckAccess();

  return z_;
}

bool Drawable::IsVisible() const {
  NeedCheckAccess();

  return visible_;
}

void Drawable::SetVisible(bool visible) {
  NeedCheckAccess();

  visible_ = visible;
}

void Drawable::UnlinkNode() {
  if (parent_) parent_->drawable_children_.Remove(node_);
}

bool Drawable::operator<(const Drawable& other) const {
  if (z_ <= other.z_) {
    return true;
  }
  return false;
}

}  // namespace modules
