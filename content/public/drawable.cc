// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/drawable.h"

namespace content {

namespace {
uint64_t g_creation_stamp = 0;
}  // namespace

Drawable::Drawable(DrawableParent* parent, int z, bool visible, int sprite_y)
    : base::LinkNode<Drawable>(),
      init_data_complete_(false),
      parent_(parent),
      z_(z),
      visible_(visible),
      creation_stamp_(++g_creation_stamp),
      sprite_y_(sprite_y) {
  parent_->InsertDrawable(this);
}

Drawable::~Drawable() {
  RemoveFromList();
}

void Drawable::SetParent(DrawableParent* parent) {
  CheckDisposed();

  if (parent_ == parent)
    return;

  RemoveFromList();
  parent_ = parent;
  parent_->InsertDrawable(this);

  OnViewportRectChanged(parent->viewport_rect());
}

void Drawable::SetZ(int z) {
  CheckDisposed();

  if (z_ == z)
    return;

  z_ = z;

  RemoveFromList();
  parent_->InsertDrawable(this);
}

void Drawable::SetSpriteY(int y) {
  sprite_y_ = y;
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
    if (CalcDrawableOrder(it->value(), drawable)) {
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
    auto* child = it->value();
    if (child->visible_) {
      // Init once for renderer data on RenderRunner
      if (!child->init_data_complete_) {
        child->InitDrawableData();
        child->init_data_complete_ = true;
      }

      // Prepare for render composite
      child->BeforeComposite();
    }
  }
}

void DrawableParent::CompositeChildren() {
  if (drawables_.empty())
    return;

  for (auto it = drawables_.tail(); it != drawables_.end();
       it = it->previous()) {
    if (it->value()->visible_)
      it->value()->Composite();
  }
}

void DrawableParent::NotifyViewportChanged() {
  for (auto it = drawables_.head(); it != drawables_.end(); it = it->next()) {
    it->value()->OnViewportRectChanged(viewport_rect_);
  }
}

bool DrawableParent::CalcDrawableOrder(Drawable* self, Drawable* other) {
  if (self->z_ < other->z_) {
    return true;
  }

  if (self->z_ == other->z_) {
    if (self->sprite_y_ == other->sprite_y_)
      return (self->creation_stamp_ < other->creation_stamp_);
    else
      return (self->sprite_y_ < other->sprite_y_);
  }

  return false;
}

}  // namespace content