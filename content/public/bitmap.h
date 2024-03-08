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

  Bitmap(scoped_refptr<Graphics> host, int width, int height);
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
  void UpdateSurface();

  base::CallbackListSubscription AddBitmapObserver(
      base::RepeatingClosure observer) {
    return observers_.Add(std::move(observer));
  }

  uint64_t GetTexID() const { return reinterpret_cast<uint64_t>(this); }

 protected:
  void OnObjectDisposed() override;
  std::string_view DisposedObjectName() const override { return "Bitmap"; }

 private:
  static void InitBitmapInternal(
      uint64_t self,
      const std::variant<base::Vec2i, SDL_Surface*>& initial_data);
  static void StretchBltInternal(uint64_t self,
                                 const base::Rect& dest_rect,
                                 uint64_t src_bitmap,
                                 const base::Rect& src_rect,
                                 float opacity);
  static void FillRectInternal(uint64_t self,
                               const base::Rect& rect,
                               const base::Vec4& color);
  static void GradientFillRectInternal(uint64_t self,
                                       const base::Rect& rect,
                                       const base::Vec4& color1,
                                       const base::Vec4& color2,
                                       bool vertical);
  static void SetPixelInternal(uint64_t self,
                               int x,
                               int y,
                               const base::Vec4& color);
  static void HueChangeInternal(uint64_t self, int hue);
  static void DrawTextInternal(uint64_t self,
                               scoped_refptr<Font> font,
                               const base::Rect& rect,
                               const std::string& str,
                               TextAlign align);
  static void GetSurfaceInternal(uint64_t self, SDL_Surface* output);
  static void UpdateSurfaceInternal(uint64_t self, SDL_Surface* output);

  void NeedUpdateSurface();

  base::Vec2i size_;
  scoped_refptr<Font> font_;
  SDL_Surface* surface_cache_;

  base::RepeatingClosureList observers_;

  base::WeakPtrFactory<Bitmap> weak_ptr_factory_{this};
};

}  // namespace content

#endif  // !CONTENT_PUBLIC_BITMAP_H_
