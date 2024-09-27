// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_GRAPHICS_H_
#define CONTENT_PUBLIC_GRAPHICS_H_

#include "components/filesystem/filesystem.h"
#include "components/fpslimiter/fpslimiter.h"
#include "content/profile/engine_profile.h"
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
  virtual bgfx::FrameBufferHandle GetScreenBuffer() = 0;
  virtual filesystem::Filesystem* GetFileIO() = 0;
};

class Graphics final : public base::RefCounted<Graphics>,
                       public DrawableParent,
                       public GraphicsHost,
                       public DisposableCollection {
 public:
  Graphics(base::WeakPtr<ui::Widget> window,
           filesystem::Filesystem* io,
           scoped_refptr<Profile> config,
           const base::Vec2i& initial_resolution);
  ~Graphics() override;

  Graphics(const Graphics&) = delete;
  Graphics& operator=(const Graphics&) = delete;

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

  void Freeze();
  void Transition(int duration = 10,
                  scoped_refptr<Bitmap> trans_bitmap = nullptr,
                  int vague = 40);

  void SetFrameRate(int rate);
  int GetFrameRate() const;
  void SetFrameCount(int64_t count);
  int GetFrameCount() const;
  void FrameReset();

  uint64_t GetWindowHandle();
  void ResizeWindow(int width, int height);

  bool GetFullscreen();
  void SetFullscreen(bool fullscreen);

  void SetVSync(int interval);
  int GetVSync();

  bool GetFrameSkip();
  void SetFrameSkip(bool skip);

  int GetDisplayWidth();
  int GetDisplayHeight();

  void SetDrawableOffset(const base::Vec2i& offset);
  base::Vec2i GetDrawableOffset();

  scoped_refptr<Profile> config() { return config_; }
  base::WeakPtr<ui::Widget> window() { return window_; }
  ScopedFontData* font_manager() { return static_font_manager_.get(); }

 protected:
  renderer::RenderDevice* device() override { return device_.get(); }
  APIVersion api_version() override { return config_->content_version(); }
  bgfx::FrameBufferHandle GetScreenBuffer() override { return screen_buffer_; }
  filesystem::Filesystem* GetFileIO() override { return io_; }

  void AddDisposable(Disposable* disp) override;
  void RemoveDisposable(Disposable* disp) override;

 private:
  void RebuildScreenBufferInternal();
  void FrameProcessInternal();
  void UpdateAverageFPSInternal();
  void UpdateWindowViewportInternal();

  base::Vec2i resolution_;
  std::unique_ptr<renderer::RenderDevice> device_;
  bgfx::FrameBufferHandle screen_buffer_;
  bgfx::FrameBufferHandle frozen_snapshot_;
  std::unique_ptr<renderer::QuadDrawable> screen_quad_;

  scoped_refptr<Profile> config_;
  base::WeakPtr<ui::Widget> window_;

  base::LinkedList<Disposable> disposable_elements_;
  std::unique_ptr<ScopedFontData> static_font_manager_;

  bool frozen_;
  int brightness_;
  uint64_t frame_count_;
  int frame_rate_;
  bool vsync_;
  std::unique_ptr<fpslimiter::FPSLimiter> fps_manager_;
  filesystem::Filesystem* io_;

  base::Rect display_viewport_;
  base::Vec2i window_size_;
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