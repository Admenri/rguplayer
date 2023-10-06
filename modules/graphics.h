// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MODULES_GRAPHICS_H_
#define MODULES_GRAPHICS_H_

#include "content/render_thread.h"
#include "modules/drawable.h"

namespace modules {

class Screen : public DrawableManager {
 public:
  Screen(scoped_refptr<content::RendererThread> render_thread,
         const base::Vec2i& resolution);
  ~Screen() override;

  Screen(const Screen&) = delete;
  Screen& operator=(const Screen&) = delete;

  // Warning: Sync methods.
  void Composite();
  void ResizeScreen(const base::Vec2i& size);

  renderer::DoubleFrameBuffer* GetScreenBuffer() {
    return double_buffer_.get();
  }

  base::Vec2i GetResolution() { return drawable_viewport_.rect_.Size(); }

 private:
  void InitScreenInternal();
  void CompositeInternal(base::OnceClosure sync_complete);
  void ResizeScreenInternal(const base::Vec2i& size);

  void SwapBuffer(renderer::CCLayer* cc);

  scoped_refptr<content::RendererThread> render_thread_;
  std::unique_ptr<renderer::QuadDrawable> screen_quad_;
  std::unique_ptr<renderer::DoubleFrameBuffer> double_buffer_;

  base::WeakPtrFactory<Screen> weak_ptr_factory_{this};
};

class Graphics {
 public:
  Graphics(scoped_refptr<content::RendererThread> render_thread,
           const base::Rect& screen_info);
  virtual ~Graphics();

  Graphics(const Graphics&) = delete;
  Graphics& operator=(const Graphics&) = delete;

  int GetWidth() const;
  int GetHeight() const;

  void Update();

  scoped_refptr<content::RendererThread> GetRenderer() {
    return render_thread_;
  }

  Screen* GetScreen() { return screen_.get(); }

 private:
  void DrawScreenInternal();

  scoped_refptr<content::RendererThread> render_thread_;

  struct {
    base::Vec2i resolution;
    base::Vec2i display_offset_;
  } screen_info_;

  std::unique_ptr<Screen> screen_;

  base::WeakPtrFactory<Graphics> weak_ptr_factory_{this};
};

}  // namespace modules

#endif  // MODULES_GRAPHICS_H_