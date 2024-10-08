// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_GRAPHICS_H_
#define CONTENT_PUBLIC_GRAPHICS_H_

#include "components/filesystem/filesystem.h"
#include "components/fpslimiter/fpslimiter.h"
#include "content/common/content_utils.h"
#include "content/public/disposable.h"
#include "content/public/drawable.h"
#include "content/public/font.h"
#include "renderer/device/render_device.h"
#include "ui/widget/widget.h"

namespace content {

class Bitmap;

class GraphicsHost {
 public:
  GraphicsHost() = default;
  virtual ~GraphicsHost() = default;

  virtual renderer::RenderDevice* device() = 0;
  virtual APIVersion api_version() = 0;
};

class Graphics final : public base::RefCounted<Graphics>,
                       public DrawableParent,
                       public GraphicsHost,
                       public DisposableCollection {
 public:
  Graphics(CoroutineContext* cc,
           base::WeakPtr<ui::Widget> window,
           std::unique_ptr<ScopedFontData> default_font,
           const base::Vec2i& initial_resolution,
           APIVersion api_diff);
  ~Graphics() override;

  Graphics(const Graphics&) = delete;
  Graphics& operator=(const Graphics&) = delete;

  bool ExecuteEventMainLoop();

  base::Vec2i GetSize() const { return screen_buffer_.size; }

  int GetBrightness() const;
  void SetBrightness(int brightness);

  void Wait(int duration);
  scoped_refptr<Bitmap> SnapToBitmap();

  void FadeOut(int duration);
  void FadeIn(int duration);

  void Update();
  void Reset();

  void Freeze();
  void Transition(int duration = 10,
                  scoped_refptr<Bitmap> trans_bitmap = nullptr,
                  int vague = 40);

  void SetFrameRate(int rate);
  int GetFrameRate() const;
  void SetFrameCount(int64_t count);
  int GetFrameCount() const;
  void ResetFrame();

  void ResizeScreen(const base::Vec2i& resolution);
  void ResizeWindow(int width, int height);

  bool GetFullscreen();
  void SetFullscreen(bool fullscreen);

  void SetVSync(int interval);
  int GetVSync();

  int GetDisplayWidth();
  int GetDisplayHeight();

  void SetDrawableOffset(const base::Vec2i& offset);
  base::Vec2i GetDrawableOffset();

  ScopedFontData* font_manager() { return static_font_manager_.get(); }

  renderer::RenderDevice* device() override { return device_.get(); }
  APIVersion api_version() override { return api_version_; }

 protected:
  void AddDisposable(Disposable* disp) override;
  void RemoveDisposable(Disposable* disp) override;

 private:
  void RebuildScreenBufferInternal(const base::Vec2i& resolution);
  void FrameProcessInternal();
  int DetermineRepeatNumberInternal(double delta_rate);
  void UpdateWindowViewportInternal();
  void EncodeScreenDrawcallsInternal(bgfx::Encoder* encoder,
                                     bgfx::ViewId* render_view);
  void PresentScreenBufferInternal(renderer::Framebuffer* buffer,
                                   bgfx::Encoder* encoder,
                                   bgfx::ViewId render_view);

  std::unique_ptr<renderer::RenderDevice> device_;
  APIVersion api_version_;

  renderer::Framebuffer screen_buffer_;
  renderer::Framebuffer frozen_snapshot_;
  std::unique_ptr<renderer::QuadDrawable> screen_quad_;

  base::LinkedList<Disposable> disposable_elements_;
  std::unique_ptr<ScopedFontData> static_font_manager_;

  base::Rect display_viewport_;
  base::Vec2i window_size_;

  bool frozen_;
  int brightness_;
  uint64_t frame_count_;
  int frame_rate_;
  bool vsync_;

  double elapsed_time_;
  double smooth_delta_time_;
  uint64_t last_count_time_;
  uint64_t desired_delta_time_;

  CoroutineContext* cc_;
};

class GraphicsElement {
 public:
  GraphicsElement(GraphicsHost* screen) : screen_(screen) {}

  GraphicsElement(const GraphicsElement&) = delete;
  GraphicsElement& operator=(const GraphicsElement&) = delete;

  inline GraphicsHost* screen() const { return screen_; }

 private:
  GraphicsHost* screen_;
};

}  // namespace content

#endif  // !CONTENT_PUBLIC_GRAPHICS_H_