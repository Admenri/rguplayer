// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MODULES_BITMAP_H_
#define MODULES_BITMAP_H_

#include <SDL_surface.h>

#include <memory>
#include <optional>
#include <string>
#include <variant>

#include "base/bind/callback.h"
#include "base/math/math.h"
#include "content/render_thread.h"
#include "modules/disposable.h"
#include "modules/utility.h"
#include "renderer/compositor/renderer_cc.h"
#include "renderer/paint/frame_buffer_canvas.h"

namespace modules {

class Bitmap : public base::RefCounted<Bitmap>, public Disposable {
 public:
  Bitmap(scoped_refptr<content::RendererThread> worker, int width, int height);
  Bitmap(scoped_refptr<content::RendererThread> worker,
         const std::string& filename);
  ~Bitmap() override;

  Bitmap(const Bitmap&) = delete;
  Bitmap& operator=(const Bitmap&) = delete;

  Bitmap(Bitmap&&) = default;

  base::Vec2i GetSize();
  base::Rect GetRect();

  scoped_refptr<Bitmap> Clone();

  void Blt(int x, int y, scoped_refptr<Bitmap> src_bitmap,
           const base::Rect& src_rect, int opacity = 255);
  void StretchBlt(const base::Rect& dst_rect, scoped_refptr<Bitmap> src_bitmap,
                  const base::Rect& src_rect, int opacity = 255);

  void FillRect(const base::Rect& rect, scoped_refptr<Color> color);

  void GradientFillRect(const base::Rect& rect, scoped_refptr<Color> color1,
                        scoped_refptr<Color> color2, bool vertical = false);

  void Clear();
  void ClearRect(const base::Rect& rect);

  scoped_refptr<Color> GetPixel(int x, int y);
  void SetPixel(int x, int y, scoped_refptr<Color> color);

  void HueChange(int hue);

  void Blur();
  void RadialBlur(int angle, int division);

  SDL_Surface* GetSurface();

  /* Called from render thread */
  scoped_refptr<renderer::GLTexture> GetGLTexture() { return texture_; }

 protected:
  void OnObjectDisposed() override;

 private:
  void NeedUpdate();
  void EnsureSurfaceFormat(SDL_Surface*& surface);
  void InitTextureFrameBuffer(
      std::variant<base::Vec2i, SDL_Surface*> init_data);
  void ClearInternal(const std::optional<base::Rect>& rect);
  void GetSurfaceInternal(base::OnceClosure complete_closure);
  void SetPixelInternal(const base::Vec2i& pos, const base::Vec4i& color);
  void StretchBltInternal(const base::Rect& dst_rect,
                          scoped_refptr<Bitmap> src_bitmap,
                          const base::Rect& src_rect, int opacity);
  void FillRectInternal(const base::Rect& rect, const base::Vec4& color);
  void GradientFillRectInternal(const base::Rect& rect,
                                const base::Vec4& color1,
                                const base::Vec4& color2, bool vertical);

  scoped_refptr<content::RendererThread> worker_;
  scoped_refptr<renderer::GLTexture> texture_;
  std::unique_ptr<renderer::GLFrameBuffer> frame_buffer_;

  SDL_Surface* read_pixel_buffer_ = nullptr;
  base::Vec2i size_;

  std::unique_ptr<SDL_PixelFormat, decltype(&SDL_FreeFormat)> pixel_format_;

  base::WeakPtrFactory<Bitmap> weak_ptr_factory_{this};
};

}  // namespace modules

#endif  // MODULES_BITMAP_H_