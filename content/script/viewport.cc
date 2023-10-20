// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/script/viewport.h"

#include "content/scheduler/worker_cc.h"

namespace content {

Viewport::Viewport() : Drawable(BindingRunner::Get()->GetScreen(), 0, true) {
  viewport_rect().rect = BindingRunner::Get()->GetScreen()->GetSize();
}

Viewport::Viewport(const base::Rect& rect)
    : Drawable(BindingRunner::Get()->GetScreen(), 0, true) {
  viewport_rect().rect = rect;
}

Viewport::~Viewport() { Dispose(); }

void Viewport::OnObjectDisposed() { RemoveFromList(); }

void Viewport::Composite() {
  if (Flashable::IsFlashEmpty()) return;

  gpu::GSM.states.scissor.Push(true);
  gpu::GSM.states.scissor_rect.Push(viewport_rect().rect);

  DrawableParent::CompositeChildren();

  gpu::GSM.states.scissor_rect.Pop();
  gpu::GSM.states.scissor.Pop();
}

void Viewport::OnViewportRectChanged(const ViewportInfo& rect) {}

ViewportChild::ViewportChild(scoped_refptr<Viewport> viewport, int z)
    : Drawable(viewport ? static_cast<DrawableParent*>(viewport.get())
                        : BindingRunner::Get()->GetScreen(),
               z, true) {}

void ViewportChild::SetViewport(scoped_refptr<Viewport> viewport) {
  CheckDisposed();

  if (viewport == viewport_) return;

  viewport_ = viewport;

  SetParent(viewport_.get());
  OnViewportChanged();
}

}  // namespace content
