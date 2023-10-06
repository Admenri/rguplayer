// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "modules/drawable.h"

#include "drawable.h"

namespace modules {

DrawableManager::DrawableManager() {}

DrawableManager::~DrawableManager() {
  for (base::LinkNode<Drawable>* it = drawable_children_.begin();
       it != drawable_children_.end(); it = it->Next()) {
    it->Data()->drawable_manager_ = nullptr;
  }
}

void DrawableManager::Composite() {
  for (base::LinkNode<Drawable>* it = drawable_children_.begin();
       it != drawable_children_.end(); it = it->Next()) {
    if (it->Data()->IsVisible()) it->Data()->Paint();
  }
}

void DrawableManager::NotifyViewportChanged() {
  for (base::LinkNode<Drawable>* it = drawable_children_.begin();
       it != drawable_children_.end(); it = it->Next()) {
    it->Data()->ViewportRectChanged(drawable_viewport_);
  }
}

void DrawableManager::Insert(Drawable& element) {
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

void DrawableManager::InsertAfter(Drawable& element, Drawable& after) {
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

void DrawableManager::Reinsert(Drawable& element) {
  drawable_children_.Remove(element.node_);
  Insert(element);
}

Drawable::Drawable(DrawableManager* frame, int z, bool visible)
    : drawable_manager_(frame), z_(z), visible_(visible), node_(this) {
  frame->Insert(*this);
}

Drawable::~Drawable() { UnlinkNode(); }

void Drawable::SetDrawableManager(DrawableManager* frame) {
  NeedCheckAccess();

  UnlinkNode();

  drawable_manager_ = frame;

  frame->Insert(*this);

  ViewportRectChanged(frame->drawable_viewport_);
}

void Drawable::SetZ(int z) {
  NeedCheckAccess();

  if (z_ == z) return;

  z_ = z;

  drawable_manager_->Reinsert(*this);
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
  if (drawable_manager_) drawable_manager_->drawable_children_.Remove(node_);
}

bool Drawable::operator<(const Drawable& other) const {
  if (z_ <= other.z_) {
    return true;
  }
  return false;
}

}  // namespace modules
