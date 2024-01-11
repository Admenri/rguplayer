// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "content/public/bitmap.h"

#include "SDL_image.h"

#include "content/worker/renderer_worker.h"
#include "renderer/quad/quad_drawable.h"

namespace content {

Bitmap::Bitmap(scoped_refptr<Graphics> host, int width, int height)
    : GraphicElement(host),
      Disposable(host),
      pixel_format_(SDL_CreatePixelFormat(SDL_PIXELFORMAT_ABGR8888)) {
  width = std::abs(width);
  height = std::abs(height);

  size_ = base::Vec2i(width, height);
  surface_buffer_ = nullptr;

  screen()->renderer()->PostTask(base::BindOnce(&Bitmap::InitBitmapInternal,
                                                weak_ptr_factory_.GetWeakPtr(),
                                                base::Vec2i(width, height)));
}

Bitmap::Bitmap(scoped_refptr<Graphics> host, const std::string& filename)
    : GraphicElement(host),
      Disposable(host),
      pixel_format_(SDL_CreatePixelFormat(SDL_PIXELFORMAT_ABGR8888)) {
  surface_buffer_ = IMG_Load(filename.c_str());

  size_ = base::Vec2i(surface_buffer_->w, surface_buffer_->h);

  if (surface_buffer_->format->format != pixel_format_->format) {
    SDL_Surface* conv = SDL_ConvertSurface(surface_buffer_, pixel_format_);
    SDL_DestroySurface(surface_buffer_);
    surface_buffer_ = conv;
  }

  screen()->renderer()->PostTask(base::BindOnce(&Bitmap::InitBitmapInternal,
                                                weak_ptr_factory_.GetWeakPtr(),
                                                surface_buffer_));
}

Bitmap::~Bitmap() {
  Dispose();
}

scoped_refptr<Bitmap> Bitmap::Clone() {
  CheckIsDisposed();

  scoped_refptr<Bitmap> new_bitmap = new Bitmap(screen(), size_.x, size_.y);
  new_bitmap->Blt(0, 0, this, size_);

  return new_bitmap;
}

void Bitmap::Blt(int x,
                 int y,
                 scoped_refptr<Bitmap> src_bitmap,
                 const base::Rect& src_rect,
                 int opacity) {
  CheckIsDisposed();

  if (src_rect.width <= 0 || src_rect.height <= 0)
    return;
  if (src_bitmap->IsDisposed() || !opacity)
    return;

  base::Rect rect = src_rect;

  if (rect.x + rect.width > src_bitmap->GetWidth())
    rect.width = src_bitmap->GetWidth() - rect.x;

  if (rect.y + rect.height > src_bitmap->GetHeight())
    rect.height = src_bitmap->GetHeight() - rect.y;

  rect.width = std::max(0, rect.width);
  rect.height = std::max(0, rect.height);

  StretchBlt(base::Rect(x, y, rect.width, rect.height), src_bitmap, rect,
             opacity);
}

void Bitmap::StretchBlt(const base::Rect& dest_rect,
                        scoped_refptr<Bitmap> src_bitmap,
                        const base::Rect& src_rect,
                        int opacity) {
  CheckIsDisposed();

  if (dest_rect.width <= 0 || dest_rect.height <= 0)
    return;
  if (src_rect.width <= 0 || src_rect.height <= 0)
    return;

  opacity = std::clamp(opacity, 0, 255);

  if (src_bitmap->IsDisposed() || !opacity)
    return;

  screen()->renderer()->PostTask(base::BindOnce(
      &Bitmap::StretchBltInternal, weak_ptr_factory_.GetWeakPtr(), dest_rect,
      src_bitmap, src_rect, opacity / 255.0f));

  NeedUpdateSurface();
}

void Bitmap::FillRect(const base::Rect& rect, scoped_refptr<Color> color) {
  CheckIsDisposed();

  if (rect.width <= 0 || rect.height <= 0)
    return;

  screen()->renderer()->PostTask(base::BindOnce(&Bitmap::FillRectInternal,
                                                weak_ptr_factory_.GetWeakPtr(),
                                                rect, color->AsBase()));

  NeedUpdateSurface();
}

void Bitmap::GradientFillRect(const base::Rect& rect,
                              scoped_refptr<Color> color1,
                              scoped_refptr<Color> color2,
                              bool vertical) {
  CheckIsDisposed();

  if (rect.width <= 0 || rect.height <= 0)
    return;

  screen()->renderer()->PostTask(base::BindOnce(
      &Bitmap::GradientFillRectInternal, weak_ptr_factory_.GetWeakPtr(), rect,
      color1->AsBase(), color2->AsBase(), vertical));

  NeedUpdateSurface();
}

void Bitmap::Clear() {
  CheckIsDisposed();

  screen()->renderer()->PostTask(base::BindOnce(&Bitmap::FillRectInternal,
                                                weak_ptr_factory_.GetWeakPtr(),
                                                size_, base::Vec4()));

  NeedUpdateSurface();
}

void Bitmap::ClearRect(const base::Rect& rect) {
  CheckIsDisposed();

  screen()->renderer()->PostTask(base::BindOnce(&Bitmap::FillRectInternal,
                                                weak_ptr_factory_.GetWeakPtr(),
                                                rect, base::Vec4()));

  NeedUpdateSurface();
}

scoped_refptr<Color> Bitmap::GetPixel(int x, int y) {
  CheckIsDisposed();

  if (surface_need_update_) {
    SurfaceRequired();
    surface_need_update_ = false;
  }

  int bpp = surface_buffer_->format->BytesPerPixel;
  uint8_t* pixel = static_cast<uint8_t*>(surface_buffer_->pixels) +
                   y * surface_buffer_->pitch + x * bpp;

  uint8_t color[4];
  uint8_t* p = color;
  SDL_GetRGBA(*reinterpret_cast<uint32_t*>(pixel), surface_buffer_->format, p,
              ++p, ++p, ++p);

  return new Color(color[0], color[1], color[2], color[3]);
}

void Bitmap::SetPixel(int x, int y, scoped_refptr<Color> color) {
  CheckIsDisposed();

  if (surface_buffer_) {
    int bpp = surface_buffer_->format->BytesPerPixel;
    uint8_t* pixel = static_cast<uint8_t*>(surface_buffer_->pixels) +
                     y * surface_buffer_->pitch + x * bpp;

    auto data = color->AsNormal();
    *reinterpret_cast<uint32_t*>(pixel) =
        SDL_MapRGBA(pixel_format_, data.x, data.y, data.z, data.w);
  }

  screen()->renderer()->PostTask(base::BindOnce(&Bitmap::SetPixelInternal,
                                                weak_ptr_factory_.GetWeakPtr(),
                                                x, y, color->AsNormal()));

  NeedUpdateSurface();
}

void Bitmap::HueChange(int hue) {
  CheckIsDisposed();

  // TODO:

  NeedUpdateSurface();
}

void Bitmap::Blur() {
  CheckIsDisposed();

  // TODO:

  NeedUpdateSurface();
}

void Bitmap::RadialBlur(int angle, int division) {
  CheckIsDisposed();

  // TODO:

  NeedUpdateSurface();
}

void Bitmap::DrawText(const base::Rect& rect,
                      const std::string& str,
                      int align) {
  CheckIsDisposed();

  // TODO:

  NeedUpdateSurface();
}

scoped_refptr<Rect> Bitmap::TextSize(const std::string& str) {
  CheckIsDisposed();

  // TODO:

  return nullptr;
}

SDL_Surface* Bitmap::SurfaceRequired() {
  /* Sync for surface operation */
  screen()->renderer()->WaitForSync();

  if (surface_buffer_) {
    SDL_DestroySurface(surface_buffer_);
  }

  surface_buffer_ = SDL_CreateSurface(size_.x, size_.y, pixel_format_->format);

  screen()->renderer()->PostTask(base::BindOnce(
      &Bitmap::GetSurfaceInternal, weak_ptr_factory_.GetWeakPtr()));
  /* Sync for surface get pixels */
  screen()->renderer()->WaitForSync();

  return surface_buffer_;
}

void Bitmap::OnObjectDisposed() {
  weak_ptr_factory_.InvalidateWeakPtrs();

  SDL_DestroyPixelFormat(pixel_format_);

  if (surface_buffer_) {
    SDL_DestroySurface(surface_buffer_);
    surface_buffer_ = nullptr;
  }

  screen()->renderer()->PostTask(base::BindOnce(
      &renderer::TextureFrameBuffer::Del, base::OwnedRef(tex_fbo_)));
}

void Bitmap::InitBitmapInternal(
    const std::variant<base::Vec2i, SDL_Surface*>& initial_data) {
  tex_fbo_ = renderer::TextureFrameBuffer::Gen();

  if (std::holds_alternative<base::Vec2i>(initial_data)) {
    auto size = std::get<base::Vec2i>(initial_data);
    surface_buffer_ = nullptr;
    renderer::TextureFrameBuffer::Alloc(tex_fbo_, size.x, size.y);
  } else if (std::holds_alternative<SDL_Surface*>(initial_data)) {
    surface_buffer_ = std::get<SDL_Surface*>(initial_data);
    renderer::TextureFrameBuffer::Alloc(tex_fbo_, surface_buffer_->w,
                                        surface_buffer_->h);
    renderer::Texture::TexImage2D(surface_buffer_->w, surface_buffer_->h,
                                  GL_RGBA, surface_buffer_->pixels);
  } else {
    NOTREACHED();
  }

  renderer::TextureFrameBuffer::LinkFrameBuffer(tex_fbo_);
}

void Bitmap::StretchBltInternal(const base::Rect& dest_rect,
                                scoped_refptr<Bitmap> src_bitmap,
                                const base::Rect& src_rect,
                                float opacity) {
  renderer::GSM.EnsureGenericTex(dest_rect.width, dest_rect.height);
  auto& dst_tex = renderer::GSM.common_tfb;

  renderer::Blt::BeginDraw(dst_tex);
  renderer::Blt::TexSource(tex_fbo_);
  renderer::Blt::EndDraw(dest_rect, dest_rect.Size());

  /*
   * (texCoord - src_offset) * src_dst_factor
   */
  base::Vec4 offset_scale;
  offset_scale.x = static_cast<float>(src_rect.x) / src_bitmap->GetWidth();
  offset_scale.y = static_cast<float>(src_rect.y) / src_bitmap->GetHeight();
  offset_scale.z =
      (static_cast<float>(src_bitmap->GetWidth()) / src_rect.width) *
      (static_cast<float>(dest_rect.width) / dst_tex.width);
  offset_scale.w =
      (static_cast<float>(src_bitmap->GetHeight()) / src_rect.height) *
      (static_cast<float>(dest_rect.height) / dst_tex.height);

  auto& shader = renderer::GSM.shaders->texblt;

  renderer::GSM.states.viewport.Push(size_);
  renderer::GSM.states.blend.Push(false);

  renderer::FrameBuffer::Bind(tex_fbo_.fbo);

  shader.Bind();
  shader.SetProjectionMatrix(size_);
  shader.SetTransOffset(base::Vec2i());
  shader.SetSrcTexture(src_bitmap->tex_fbo_.tex);
  shader.SetTextureSize(src_bitmap->size_);
  shader.SetDstTexture(dst_tex.tex);
  shader.SetOffsetScale(offset_scale);
  shader.SetOpacity(opacity);

  auto* quad = renderer::GSM.common_quad.get();
  quad->SetPositionRect(dest_rect);
  quad->SetTexCoordRect(src_rect);
  quad->Draw();

  renderer::GSM.states.blend.Pop();
  renderer::GSM.states.viewport.Pop();
}

void Bitmap::FillRectInternal(const base::Rect& rect, const base::Vec4& color) {
  renderer::FrameBuffer::Bind(tex_fbo_.fbo);

  renderer::GSM.states.scissor.Push(true);
  renderer::GSM.states.scissor_rect.Push(rect);

  renderer::GSM.states.clear_color.Push(color);
  renderer::FrameBuffer::Clear();
  renderer::GSM.states.clear_color.Pop();

  renderer::GSM.states.scissor_rect.Pop();
  renderer::GSM.states.scissor.Pop();
}

void Bitmap::GradientFillRectInternal(const base::Rect& rect,
                                      const base::Vec4& color1,
                                      const base::Vec4& color2,
                                      bool vertical) {
  renderer::FrameBuffer::Bind(tex_fbo_.fbo);

  renderer::GSM.states.viewport.Push(size_);
  renderer::GSM.states.blend.Push(false);

  auto& shader = renderer::GSM.shaders->color;
  shader.Bind();
  shader.SetProjectionMatrix(size_);
  shader.SetTransOffset(base::Vec2i());

  auto* quad = renderer::GSM.common_quad.get();
  quad->SetPositionRect(rect);

  if (vertical) {
    quad->SetColor(0, color1);
    quad->SetColor(1, color1);
    quad->SetColor(2, color2);
    quad->SetColor(3, color2);
  } else {
    quad->SetColor(0, color1);
    quad->SetColor(1, color2);
    quad->SetColor(2, color2);
    quad->SetColor(3, color1);
  }

  quad->Draw();

  renderer::GSM.states.viewport.Pop();
  renderer::GSM.states.blend.Pop();
}

void Bitmap::SetPixelInternal(int x, int y, const base::Vec4i& color) {
  std::array<uint8_t, 4> pixel = {
      static_cast<uint8_t>(color.x), static_cast<uint8_t>(color.y),
      static_cast<uint8_t>(color.z), static_cast<uint8_t>(color.w)};

  renderer::Texture::Bind(tex_fbo_.tex);
  renderer::Texture::TexSubImage2D(x, y, 1, 1, GL_RGBA, pixel.data());
}

void Bitmap::GetSurfaceInternal() {
  renderer::GSM.states.viewport.Push(size_);
  renderer::FrameBuffer::Bind(tex_fbo_.fbo);
  renderer::GL.ReadPixels(0, 0, size_.x, size_.y, GL_RGBA, GL_UNSIGNED_BYTE,
                          surface_buffer_->pixels);
  renderer::GSM.states.viewport.Pop();
}

void Bitmap::NeedUpdateSurface() {
  // For get pixel cache
  surface_need_update_ = true;

  observers_.Notify();
}

}  // namespace content
