// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_SCRIPT_BITMAP_H_
#define CONTENT_SCRIPT_BITMAP_H_

#include <SDL_pixels.h>
#include <SDL_surface.h>

#include <memory>
#include <string>
#include <variant>

#include "base/math/math.h"
#include "base/memory/ref_counted.h"
#include "content/script/disposable.h"
#include "content/script/utility.h"
#include "gpu/gles2/gsm/gles_gsm.h"

namespace content {

class Bitmap : public base::RefCounted<Bitmap>, public Disposable {
 public:
  Bitmap(int width, int height);
  Bitmap(const std::string& filename);
  ~Bitmap() override;

  Bitmap(const Bitmap&) = delete;
  Bitmap& operator=(const Bitmap&) = delete;

  scoped_refptr<Bitmap> Clone();

  int GetWidth() { return size_.x; }
  int GetHeight() { return size_.y; }
  scoped_refptr<Rect> GetRect() { return new Rect(size_); }

  void Blt(int x, int y, scoped_refptr<Bitmap> src_bitmap,
           const base::Rect& src_rect, int opacity = 255);
  void StretchBlt(const base::Rect& dest_rect, scoped_refptr<Bitmap> src_bitmap,
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

  void DrawText(const base::Rect& rect, const std::string& str, int align = 0);
  scoped_refptr<Rect> TextSize(const std::string& str);

  SDL_Surface* SurfaceRequired();

  base::CallbackListSubscription AddBitmapObserver(base::OnceClosure observer) {
    return observers_.Add(std::move(observer));
  }

 protected:
  void OnObjectDisposed() override;

  std::string_view DisposedObjectName() { return "Bitmap"; }

 private:
  void InitBitmapInternal(
      const std::variant<base::Vec2i, SDL_Surface*>& initial_data);
  void DestroyBitmapInternal();
  void StretchBltInternal(const base::Rect& dest_rect,
                          scoped_refptr<Bitmap> src_bitmap,
                          const base::Rect& src_rect, float opacity);
  void FillRectInternal(const base::Rect& rect, const base::Vec4& color);
  void GradientFillRectInternal(const base::Rect& rect,
                                const base::Vec4& color1,
                                const base::Vec4& color2, bool vertical);
  void GetSurfaceInternal();
  void SetPixelInternal(int x, int y, const base::Vec4i& color);
  void NeedUpdateSurface();

  gpu::TextureFrameBuffer tex_fbo_;

  SDL_Surface* surface_buffer_;
  bool surface_need_update_ = false;

  base::Vec2i size_;
  base::OnceClosureList observers_;
  SDL_PixelFormat* pixel_format_;

  base::WeakPtrFactory<Bitmap> weak_ptr_factory_{this};
};

}  // namespace content

#endif  // CONTENT_SCRIPT_BITMAP_H_