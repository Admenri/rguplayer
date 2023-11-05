// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_SCRIPT_GRAPHICS_H_
#define CONTENT_SCRIPT_GRAPHICS_H_

#include "base/memory/weak_ptr.h"
#include "content/script/drawable.h"
#include "gpu/gles2/gsm/gles_gsm.h"
#include "ui/widget/widget.h"

namespace content {

class Graphics final : public DrawableParent {
 public:
  Graphics(base::WeakPtr<ui::Widget> window,
           const base::Vec2i& initial_resolution);
  ~Graphics();

  Graphics(const Graphics&) = delete;
  Graphics& operator=(const Graphics&) = delete;

  int GetWidth() const { return resolution_.x; }
  int GetHeight() const { return resolution_.y; }
  base::Vec2i GetSize() { return resolution_; }

  void Update();

 private:
  void InitScreenBufferInternal();
  void DestroyBufferInternal();
  void CompositeScreenInternal();
  void ResizeResolutionInternal();

  void PresentScreenInternal(bool* paint_raiser);

  gpu::TextureFrameBuffer screen_buffer_[2];
  std::unique_ptr<gpu::QuadDrawable> screen_quad_;

  base::WeakPtr<ui::Widget> window_;
  base::Vec2i resolution_;

  uint64_t frame_count_ = 0;
  double frame_rate_ = 60.0;

  base::WeakPtrFactory<Graphics> weak_ptr_factory_{this};
};

}  // namespace content

#endif  // CONTENT_SCRIPT_GRAPHICS_H_