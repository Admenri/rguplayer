// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/viewport.h"

namespace content {

Viewport::Viewport(scoped_refptr<Graphics> screen)
    : GraphicElement(screen), Drawable(screen.get(), 0, true) {
  viewport_rect().rect = screen->GetSize();
}

Viewport::Viewport(scoped_refptr<Graphics> screen, const base::Rect& rect)
    : GraphicElement(screen), Drawable(screen.get(), 0, true) {
  viewport_rect().rect = rect;
}

Viewport::~Viewport() {
  Dispose();
}

void Viewport::SetOX(int ox) {
  CheckDisposed();

  viewport_rect().origin.x = ox;
  NotifyViewportChanged();
}

void Viewport::SetOY(int oy) {
  CheckDisposed();

  viewport_rect().origin.y = oy;
  NotifyViewportChanged();
}

void Viewport::OnObjectDisposed() {
  RemoveFromList();
}

void Viewport::Composite() {
  if (Flashable::IsFlashEmpty())
    return;

  renderer::GSM.states.scissor.Push(true);
  renderer::GSM.states.scissor_rect.Push(viewport_rect().rect);

  DrawableParent::CompositeChildren();

  renderer::GSM.states.scissor_rect.Pop();
  renderer::GSM.states.scissor.Pop();
}

void Viewport::OnViewportRectChanged(const ViewportInfo& rect) {}

ViewportChild::ViewportChild(scoped_refptr<Graphics> screen,
                             scoped_refptr<Viewport> viewport,
                             int z)
    : Drawable(viewport ? static_cast<DrawableParent*>(viewport.get())
                        : screen.get(),
               z,
               true) {}

void ViewportChild::SetViewport(scoped_refptr<Viewport> viewport) {
  CheckDisposed();

  if (viewport == viewport_)
    return;

  viewport_ = viewport;

  SetParent(viewport_.get());
  OnViewportChanged();
}

}  // namespace content
