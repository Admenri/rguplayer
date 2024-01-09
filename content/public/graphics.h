// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_GRAPHICS_H_
#define CONTENT_PUBLIC_GRAPHICS_H_

#include "base/memory/weak_ptr.h"
#include "content/public/drawable.h"
#include "content/worker/renderer_worker.h"
#include "renderer/thread/thread_manager.h"

namespace content {

class Bitmap;
class Disposable;

class Graphics final : public base::RefCounted<Graphics>,
                       public DrawableParent {
 public:
  Graphics(scoped_refptr<RenderRunner> renderer,
           const base::Vec2i& initial_resolution);
  ~Graphics();

  Graphics(const Graphics&) = delete;
  Graphics& operator=(const Graphics&) = delete;

  int GetWidth() const { return resolution_.x; }
  int GetHeight() const { return resolution_.y; }
  base::Vec2i GetSize() { return resolution_; }

  int GetBrightness() const;
  void SetBrightness(int brightness);

  void Wait(int duration);
  scoped_refptr<Bitmap> SnapToBitmap();

  void FadeOut(int duration);
  void FadeIn(int duration);

  void Update();
  void ResizeScreen(const base::Vec2i& resolution);
  void Reset();

  scoped_refptr<RenderRunner> renderer() { return renderer_; }

 private:
  friend class Viewport;
  friend class Disposable;

  void InitScreenBufferInternal();
  void DestroyBufferInternal();
  void CompositeScreenInternal();
  void ResizeResolutionInternal();
  void PresentScreenInternal(bool* paint_raiser);
  void SetBrightnessInternal();
  void SnapToBitmapInternal(scoped_refptr<Bitmap> target);

  void AddDisposable(Disposable* disp);
  void RemoveDisposable(Disposable* disp);

  void RenderEffectRequire(const base::Vec4& color,
                           const base::Vec4& tone,
                           const base::Vec4& flash_color);

  renderer::TextureFrameBuffer screen_buffer_[2];
  std::unique_ptr<renderer::QuadDrawable> screen_quad_;
  std::unique_ptr<renderer::QuadDrawable> brightness_quad_;

  scoped_refptr<RenderRunner> renderer_;
  base::Vec2i resolution_;
  base::LinkedList<Disposable> disposable_elements_;

  uint64_t frame_count_ = 0;
  double frame_rate_ = 60.0;
  int brightness_ = 255;

  base::WeakPtrFactory<Graphics> weak_ptr_factory_{this};
};

class GraphicElement {
 public:
  GraphicElement(scoped_refptr<Graphics> screen) : host_screen_(screen) {}
  virtual ~GraphicElement() = default;

  GraphicElement(const GraphicElement&) = delete;
  GraphicElement& operator=(const GraphicElement&) = delete;

  scoped_refptr<Graphics> screen() { return host_screen_; }

 private:
  scoped_refptr<Graphics> host_screen_;
};

}  // namespace content

#endif  // !CONTENT_PUBLIC_GRAPHICS_H_