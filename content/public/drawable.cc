// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/drawable.h"

namespace content {

Drawable::Drawable(DrawableParent* parent, int z, bool visible)
    : base::LinkNode<Drawable>(), parent_(parent), z_(z), visible_(visible) {
  parent_->InsertDrawable(this);
}

Drawable::~Drawable() {
  if (parent_)
    RemoveFromList();
}

void Drawable::SetParent(DrawableParent* parent) {
  CheckDisposed();

  if (parent_)
    RemoveFromList();

  parent_ = parent;
  parent_->InsertDrawable(this);

  OnViewportRectChanged(parent->viewport_rect());
}

void Drawable::SetZ(int z) {
  CheckDisposed();

  if (!parent_ || z_ == z)
    return;

  z_ = z;

  RemoveFromList();
  parent_->InsertDrawable(this);
}

DrawableParent::DrawableParent() {}

DrawableParent::~DrawableParent() {
  for (auto it = drawables_.head(); it != drawables_.end(); it = it->next()) {
    it->value()->parent_ = nullptr;
  }
}

void DrawableParent::InsertDrawable(Drawable* drawable) {
  for (auto it = drawables_.head(); it != drawables_.end(); it = it->next()) {
    // TODO: RGSS 1/2/3 specific process
    if (it->value()->z_ <= drawable->z_) {
      return drawables_.InsertBefore(it, drawable);
    }
  }

  drawables_.Append(drawable);
}

void DrawableParent::NotifyPrepareComposite() {
  if (drawables_.empty())
    return;

  for (auto it = drawables_.tail(); it != drawables_.end();
       it = it->previous()) {
    if (it->value()->GetVisible())
      it->value()->BeforeComposite();
  }
}

void DrawableParent::CompositeChildren() {
  if (drawables_.empty())
    return;

  for (auto it = drawables_.tail(); it != drawables_.end();
       it = it->previous()) {
    if (it->value()->GetVisible())
      it->value()->Composite();
  }
}

void DrawableParent::NotifyViewportChanged() {
  for (auto it = drawables_.head(); it != drawables_.end(); it = it->next()) {
    it->value()->OnViewportRectChanged(viewport_rect_);
  }
}

}  // namespace content