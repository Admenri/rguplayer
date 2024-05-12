// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_GRAPHICS_H_
#define CONTENT_PUBLIC_GRAPHICS_H_

#include "base/memory/weak_ptr.h"
#include "components/filesystem/filesystem.h"
#include "content/config/core_config.h"
#include "content/public/drawable.h"
#include "content/public/font.h"
#include "content/worker/renderer_worker.h"
#include "renderer/thread/thread_manager.h"

namespace fpslimiter {
class FPSLimiter;
}

namespace content {

class Bitmap;
class Shader;
class Disposable;
class BindingRunner;

class Graphics final : public base::RefCounted<Graphics>,
                       public DrawableParent {
 public:
  Graphics(WorkerShareData* share_data,
           base::WeakPtr<BindingRunner> dispatcher,
           scoped_refptr<RenderRunner> renderer,
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

  inline bool frozen() const { return frozen_; }
  RGSSVersion content_version() const;
  scoped_refptr<CoreConfigure> config() { return config_; }
  scoped_refptr<RenderRunner> renderer() const { return renderer_; }
  base::WeakPtr<ui::Widget> window() { return renderer_->window(); }
  int max_texture_size() const { return renderer_->max_texture_size(); }
  filesystem::Filesystem* filesystem();

 private:
  friend class Viewport;
  friend class Disposable;

  void InitScreenBufferInternal();
  void DestroyBufferInternal();
  void CompositeScreenInternal();
  void ResizeResolutionInternal();
  void PresentScreenInternal(const renderer::TextureFrameBuffer& screen_buffer);
  void SnapToBitmapInternal(scoped_refptr<Bitmap> target);
  void FreezeSceneInternal();
  void TransitionSceneInternal(int duration, bool has_trans, int vague);
  void TransitionSceneInternalLoop(int i,
                                   int duration,
                                   scoped_refptr<Bitmap> trans_bitmap);
  void FrameProcessInternal();

  void AddDisposable(Disposable* disp);
  void RemoveDisposable(Disposable* disp);

  void RenderEffectRequire(const base::Vec4& color,
                           const base::Vec4& tone,
                           scoped_refptr<Shader> program);
  void ApplyViewportEffect(renderer::TextureFrameBuffer& frontend,
                           renderer::TextureFrameBuffer& backend,
                           renderer::QuadDrawable& quad,
                           const base::Vec4& color,
                           const base::Vec4& tone,
                           scoped_refptr<Shader> program);

  void UpdateAverageFPSInternal();
  void UpdateWindowViewportInternal();

  void SetSwapIntervalInternal();
  void CheckSyncPoint();

  void DrawGUIInternal();

  WorkerShareData* share_data_;
  renderer::TextureFrameBuffer screen_buffer_[2];
  renderer::TextureFrameBuffer frozen_snapshot_;
  std::unique_ptr<renderer::QuadDrawable> screen_quad_;

  scoped_refptr<CoreConfigure> config_;
  base::WeakPtr<BindingRunner> dispatcher_;
  scoped_refptr<RenderRunner> renderer_;
  base::Vec2i resolution_;
  base::LinkedList<Disposable> disposable_elements_;

  bool frozen_;
  int brightness_;
  uint64_t frame_count_;
  int frame_rate_;
  std::unique_ptr<fpslimiter::FPSLimiter> fps_manager_;

  base::Rect display_viewport_;
  base::Vec2i window_size_;
  int vsync_interval_;
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