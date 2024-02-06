// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_BITMAP_H_
#define CONTENT_PUBLIC_BITMAP_H_

#include "SDL_pixels.h"
#include "SDL_surface.h"

#include <memory>
#include <string>
#include <variant>

#include "base/math/math.h"
#include "base/memory/ref_counted.h"
#include "content/public/disposable.h"
#include "content/public/graphics.h"
#include "content/public/utility.h"
#include "renderer/thread/thread_manager.h"

namespace content {

class Bitmap : public base::RefCounted<Bitmap>,
               public GraphicElement,
               public Disposable {
 public:
  using TextAlign = enum {
    Left = 0,
    Center,
    Right,
  };

  /* [EXC]: invalid create size */
  Bitmap(scoped_refptr<Graphics> host, int width, int height);
  /* [EXC]: load invalid image */
  Bitmap(scoped_refptr<Graphics> host, const std::string& filename);
  ~Bitmap() override;

  Bitmap(const Bitmap&) = delete;
  Bitmap& operator=(const Bitmap&) = delete;

  scoped_refptr<Bitmap> Clone();

  int GetWidth() { return size_.x; }
  int GetHeight() { return size_.y; }
  base::Vec2i GetSize() const { return size_; }
  scoped_refptr<Rect> GetRect() { return new Rect(size_); }

  void Blt(int x,
           int y,
           scoped_refptr<Bitmap> src_bitmap,
           const base::Rect& src_rect,
           int opacity = 255);
  void StretchBlt(const base::Rect& dest_rect,
                  scoped_refptr<Bitmap> src_bitmap,
                  const base::Rect& src_rect,
                  int opacity = 255);

  void FillRect(const base::Rect& rect, scoped_refptr<Color> color);

  void GradientFillRect(const base::Rect& rect,
                        scoped_refptr<Color> color1,
                        scoped_refptr<Color> color2,
                        bool vertical = false);

  void Clear();
  void ClearRect(const base::Rect& rect);

  /* Sync Method */
  scoped_refptr<Color> GetPixel(int x, int y);
  void SetPixel(int x, int y, scoped_refptr<Color> color);

  void HueChange(int hue);
  void Blur();
  void RadialBlur(int angle, int division);

  void DrawText(const base::Rect& rect,
                const std::string& str,
                TextAlign align = TextAlign::Left);
  scoped_refptr<Rect> TextSize(const std::string& str);

  scoped_refptr<Font> GetFont() const;
  void SetFont(scoped_refptr<Font> font);

  /* Sync Method */
  SDL_Surface* SurfaceRequired();

  base::CallbackListSubscription AddBitmapObserver(
      base::RepeatingClosure observer) {
    return observers_.Add(std::move(observer));
  }

  renderer::TextureFrameBuffer& AsGLType() { return tex_fbo_; }

 protected:
  void OnObjectDisposed() override;
  std::string_view DisposedObjectName() const override { return "Bitmap"; }

 private:
  void InitBitmapInternal(
      const std::variant<base::Vec2i, SDL_Surface*>& initial_data);
  void StretchBltInternal(const base::Rect& dest_rect,
                          scoped_refptr<Bitmap> src_bitmap,
                          const base::Rect& src_rect,
                          float opacity);
  void FillRectInternal(const base::Rect& rect, const base::Vec4& color);
  void GradientFillRectInternal(const base::Rect& rect,
                                const base::Vec4& color1,
                                const base::Vec4& color2,
                                bool vertical);
  void GetSurfaceInternal();
  void SetPixelInternal(int x, int y, const base::Vec4& color);
  void HueChangeInternal(int hue);
  void DrawTextInternal(const base::Rect& rect,
                        const std::string& str,
                        TextAlign align);
  void NeedUpdateSurface();

  base::Vec2i size_;
  renderer::TextureFrameBuffer tex_fbo_;
  scoped_refptr<Font> font_;

  SDL_Surface* surface_buffer_;
  bool surface_need_update_;

  base::RepeatingClosureList observers_;

  base::WeakPtrFactory<Bitmap> weak_ptr_factory_{this};
};

}  // namespace content

#endif  // !CONTENT_PUBLIC_BITMAP_H_
