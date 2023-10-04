// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "modules/bitmap.h"

#include <SDL_image.h>

#include <algorithm>

namespace modules {

Bitmap::Bitmap(scoped_refptr<content::RendererThread> worker, int width,
               int height)
    : worker_(worker),
      pixel_format_(SDL_AllocFormat(SDL_PIXELFORMAT_ABGR8888), SDL_FreeFormat) {
  width = std::clamp(width, 1, worker->GetCC()->GetTextureMaxSize());
  height = std::clamp(height, 1, worker->GetCC()->GetTextureMaxSize());

  size_.x = width;
  size_.y = height;

  worker_->GetRenderThreadRunner()->PostTask(base::BindOnce(
      &Bitmap::InitTextureFrameBuffer, weak_ptr_factory_.GetWeakPtr(),
      base::Vec2i(width, height)));
}

Bitmap::Bitmap(scoped_refptr<content::RendererThread> worker,
               const std::string& filename)
    : worker_(worker),
      pixel_format_(SDL_AllocFormat(SDL_PIXELFORMAT_ABGR8888), SDL_FreeFormat) {
  SDL_Surface* surf = IMG_Load(filename.c_str());

  if (!surf) {
    base::Debug() << "[Core] Cannot load image:" << filename;
    throw base::Exception(base::Exception::RGSSError,
                          "Failed to load image: %s", filename.c_str());
  }

  size_.x = surf->w;
  size_.y = surf->h;

  worker_->GetRenderThreadRunner()->PostTask(base::BindOnce(
      &Bitmap::InitTextureFrameBuffer, weak_ptr_factory_.GetWeakPtr(), surf));
}

Bitmap::~Bitmap() { Dispose(); }

base::Vec2i Bitmap::GetSize() {
  CheckedForDispose();

  return size_;
}

base::Rect Bitmap::GetRect() {
  CheckedForDispose();

  return base::Rect(base::Vec2i(), size_);
}

void Bitmap::Blt(int x, int y, Bitmap* src_bitmap, const base::Rect& src_rect,
                 int opacity) {
  CheckedForDispose();

  if (src_rect.width <= 0 || src_rect.height <= 0) return;

  base::Rect rect(src_rect);

  if (src_rect.x + src_rect.width > src_bitmap->GetSize().x)
    rect.width = src_bitmap->GetSize().x - rect.x;
  if (src_rect.y + src_rect.height > src_bitmap->GetSize().y)
    rect.height = src_bitmap->GetSize().y - rect.y;

  StretchBlt(base::Rect(x, y, rect.width, rect.height), src_bitmap, src_rect,
             opacity);
}

void Bitmap::StretchBlt(const base::Rect& dst_rect, Bitmap* src_bitmap,
                        const base::Rect& src_rect, int opacity) {
  CheckedForDispose();

  if (src_rect.width <= 0 || src_rect.height <= 0) return;
  if (dst_rect.width <= 0 || dst_rect.height <= 0) return;

  opacity = std::clamp(opacity, 0, 255);
  if (!opacity) return;

  worker_->GetRenderThreadRunner()->PostTask(base::BindOnce(
      &Bitmap::StretchBltInternal, weak_ptr_factory_.GetWeakPtr(), dst_rect,
      src_bitmap, src_rect, opacity));
}

void Bitmap::FillRect(const base::Rect& rect, const Color& color) {
  CheckedForDispose();

  if (rect.width <= 0 || rect.height <= 0) return;

  worker_->GetRenderThreadRunner()->PostTask(
      base::BindOnce(&Bitmap::FillRectInternal, weak_ptr_factory_.GetWeakPtr(),
                     rect, color.ToFloatColor()));
}

void Bitmap::GradientFillRect(const base::Rect& rect, const Color& color1,
                              const Color& color2, bool vertical) {
  CheckedForDispose();

  if (rect.width <= 0 || rect.height <= 0) return;

  if (color1.GetAlpha() == 0 && color2.GetAlpha() == 0) return;

  worker_->GetRenderThreadRunner()->PostTask(base::BindOnce(
      &Bitmap::GradientFillRectInternal, weak_ptr_factory_.GetWeakPtr(), rect,
      color1.ToFloatColor(), color2.ToFloatColor(), vertical));
}

void Bitmap::Clear() {
  CheckedForDispose();

  worker_->GetRenderThreadRunner()->PostTask(base::BindOnce(
      &Bitmap::ClearInternal, weak_ptr_factory_.GetWeakPtr(), std::nullopt));
}

void Bitmap::ClearRect(const base::Rect& rect) {
  CheckedForDispose();

  worker_->GetRenderThreadRunner()->PostTask(base::BindOnce(
      &Bitmap::ClearInternal, weak_ptr_factory_.GetWeakPtr(), rect));
}

Color Bitmap::GetPixel(int x, int y) {
  CheckedForDispose();

  GetSurface();

  size_t offset =
      x * pixel_format_->BytesPerPixel + y * read_pixel_buffer_->pitch;
  uint8_t* bytes = (uint8_t*)read_pixel_buffer_->pixels + offset;
  uint32_t pixel = *((uint32_t*)bytes);

  return Color((pixel >> pixel_format_->Rshift) & 0xFF,
               (pixel >> pixel_format_->Gshift) & 0xFF,
               (pixel >> pixel_format_->Bshift) & 0xFF,
               (pixel >> pixel_format_->Ashift) & 0xFF);
}

void Bitmap::SetPixel(int x, int y, const Color& color) {
  CheckedForDispose();

  if (read_pixel_buffer_) {
    size_t offset =
        x * pixel_format_->BytesPerPixel + y * read_pixel_buffer_->pitch;
    uint8_t* bytes = (uint8_t*)read_pixel_buffer_->pixels + offset;
    *((uint32_t*)bytes) =
        SDL_MapRGBA(pixel_format_.get(), color.GetRed(), color.GetGreen(),
                    color.GetBlue(), color.GetAlpha());
  }

  worker_->GetRenderThreadRunner()->PostTask(
      base::BindOnce(&Bitmap::SetPixelInternal, weak_ptr_factory_.GetWeakPtr(),
                     base::Vec2i(x, y), color));
}

void Bitmap::HueChange(int hue) { CheckedForDispose(); }

void Bitmap::Blur() { CheckedForDispose(); }

void Bitmap::RadialBlur(int angle, int division) { CheckedForDispose(); }

SDL_Surface* Bitmap::GetSurface() {
  CheckedForDispose();

  if (!read_pixel_buffer_) {
    base::RunLoop sync_loop;
    worker_->GetRenderThreadRunner()->PostTask(base::BindOnce(
        &Bitmap::GetSurfaceInternal, weak_ptr_factory_.GetWeakPtr(),
        sync_loop.QuitClosure()));
    sync_loop.Run();
  }

  return read_pixel_buffer_;
}

void Bitmap::OnObjectDisposed() {
  worker_->GetRenderThreadRunner()->ReleaseSoon(std::move(texture_));
  worker_->GetRenderThreadRunner()->DeleteSoon(std::move(frame_buffer_));

  if (read_pixel_buffer_) SDL_FreeSurface(read_pixel_buffer_);
}

void Bitmap::NeedUpdate() {
  if (read_pixel_buffer_) {
    SDL_FreeSurface(read_pixel_buffer_);
    read_pixel_buffer_ = nullptr;
  }
}

void Bitmap::EnsureSurfaceFormat(SDL_Surface*& surface) {
  if (surface->format->format == pixel_format_->format) return;

  SDL_Surface* conv_surf =
      SDL_ConvertSurfaceFormat(surface, pixel_format_->format, 0);
  SDL_FreeSurface(surface);

  surface = conv_surf;
}

void Bitmap::InitTextureFrameBuffer(
    std::variant<base::Vec2i, SDL_Surface*> init_data) {
  auto glctx = content::RendererThread::GetCCForRenderer()->GetContext();

  frame_buffer_.reset(new renderer::GLFrameBuffer(glctx));
  texture_ = new renderer::GLTexture(glctx);
  texture_->Bind();
  texture_->SetTextureFilter(GL_NEAREST);

  if (std::holds_alternative<base::Vec2i>(init_data)) {
    texture_->SetSize(std::get<base::Vec2i>(init_data));
    texture_->AllocEmpty();
  } else if (std::holds_alternative<SDL_Surface*>(init_data)) {
    auto surf = std::get<SDL_Surface*>(init_data);
    texture_->SetSize(base::Vec2i(surf->w, surf->h));
    EnsureSurfaceFormat(surf);
    texture_->BufferData(surf->pixels, GL_RGBA);

    SDL_FreeSurface(surf);
  } else {
    NOTREACHED();
  }

  frame_buffer_->Bind();
  frame_buffer_->SetRenderTarget(texture_);
  frame_buffer_->Unbind();
}

void Bitmap::ClearInternal(const std::optional<base::Rect>& rect) {
  auto* cc = content::RendererThread::GetCCForRenderer();

  frame_buffer_->Bind();

  if (rect.has_value()) {
    base::Rect calc_rect(*rect);

    if (calc_rect.width < 0) {
      calc_rect.width = -calc_rect.width;
      calc_rect.x -= calc_rect.width;
    }

    if (calc_rect.height < 0) {
      calc_rect.height = -calc_rect.height;
      calc_rect.y -= calc_rect.height;
    }

    cc->States()->scissor_test->Push(true);
    cc->States()->scissor_region->Push(calc_rect);
  }

  frame_buffer_->Clear();

  if (rect.has_value()) {
    cc->States()->scissor_test->Pop();
    cc->States()->scissor_region->Pop();
  }

  frame_buffer_->Unbind();

  NeedUpdate();
}

void Bitmap::GetSurfaceInternal(base::OnceClosure complete_closure) {
  auto* cc = content::RendererThread::GetCCForRenderer();

  if (read_pixel_buffer_) SDL_FreeSurface(read_pixel_buffer_);
  auto size = texture_->GetSize();
  read_pixel_buffer_ = SDL_CreateRGBSurface(
      0, size.x, size.y, pixel_format_->BitsPerPixel, pixel_format_->Rmask,
      pixel_format_->Gmask, pixel_format_->Bmask, pixel_format_->Amask);

  frame_buffer_->Bind();
  cc->States()->viewport->Push(base::Rect(base::Vec2i(), size));

  frame_buffer_->ReadPixels(base::Vec4i(0, 0, size.x, size.y), GL_RGBA,
                            GL_UNSIGNED_BYTE, read_pixel_buffer_->pixels);

  cc->States()->viewport->Pop();
  frame_buffer_->Unbind();

  std::move(complete_closure).Run();
}

void Bitmap::SetPixelInternal(const base::Vec2i& pos, const Color& color) {
  texture_->Bind();

  uint32_t pixel =
      SDL_MapRGBA(pixel_format_.get(), color.GetRed(), color.GetGreen(),
                  color.GetBlue(), color.GetAlpha());
  texture_->BufferData(base::Vec4i(pos.x, pos.y, 1, 1), &pixel, GL_RGBA);

  texture_->Unbind();

  NeedUpdate();
}

void Bitmap::StretchBltInternal(const base::Rect& dst_rect, Bitmap* src_bitmap,
                                const base::Rect& src_rect, int opacity) {
  auto* cc = content::RendererThread::GetCCForRenderer();

  auto dst_size = base::Vec2i(dst_rect.width, dst_rect.height);

  cc->ResizeReusedTextureIfNeed(dst_size);
  renderer::GLFrameBuffer::BltBegin(cc, cc->ReusedFrameBuffer(), dst_size);
  renderer::GLFrameBuffer::BltSource(cc, texture_);
  renderer::GLFrameBuffer::BltEnd(
      cc, cc->ReusedFrameBuffer(), dst_rect,
      base::Rect(0, 0, dst_rect.width, dst_rect.height));

  base::Vec4 bltSubRect(
      (float)src_rect.x / src_bitmap->GetSize().x,
      (float)src_rect.y / src_bitmap->GetSize().y,
      ((float)src_bitmap->GetSize().x / src_rect.width) *
          ((float)dst_rect.width / cc->ReusedTexture()->GetSize().x),
      ((float)src_bitmap->GetSize().y / src_rect.height) *
          ((float)dst_rect.height / cc->ReusedTexture()->GetSize().y));

  cc->States()->blend->Push(false);

  frame_buffer_->Bind();
  cc->States()->viewport->Push(GetRect());

  auto* shader = cc->Shaders()->blt_shader.get();
  shader->Bind();
  shader->SetViewportMatrix(GetSize());

  shader->SetTexture(src_bitmap->GetGLTexture()->GetTextureRaw());
  shader->SetTextureSize(src_bitmap->GetSize());

  shader->SetDstTexture(cc->ReusedTexture()->GetTextureRaw());
  shader->SetSubRect(bltSubRect);
  shader->SetTransOffset(base::Vec2());
  shader->SetOpacity(opacity / 255.0f);

  auto* quad = cc->GetQuad();
  quad->SetPosition(dst_rect);
  quad->SetTexcoord(src_rect);
  quad->Draw();

  cc->States()->viewport->Pop();
  frame_buffer_->Unbind();

  cc->States()->blend->Pop();

  NeedUpdate();
}

void Bitmap::FillRectInternal(const base::Rect& rect, const base::Vec4& color) {
  auto* cc = content::RendererThread::GetCCForRenderer();

  base::Rect calc_rect(rect);
  if (calc_rect.width < 0) {
    calc_rect.width = -calc_rect.width;
    calc_rect.x -= calc_rect.width;
  }

  if (calc_rect.height < 0) {
    calc_rect.height = -calc_rect.height;
    calc_rect.y -= calc_rect.height;
  }

  cc->States()->scissor_test->Push(true);
  cc->States()->scissor_region->Push(calc_rect);

  frame_buffer_->Bind();
  frame_buffer_->Clear(color);
  frame_buffer_->Unbind();

  cc->States()->scissor_region->Pop();
  cc->States()->scissor_test->Pop();

  NeedUpdate();
}

void Bitmap::GradientFillRectInternal(const base::Rect& rect,
                                      const base::Vec4& color1,
                                      const base::Vec4& color2, bool vertical) {
  auto* cc = content::RendererThread::GetCCForRenderer();

  auto* quad = cc->GetQuad();
  quad->SetPosition(rect);

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

  frame_buffer_->Bind();
  cc->States()->viewport->Push(base::Rect(base::Vec2i(), GetSize()));
  cc->States()->blend->Push(false);

  auto* shader = cc->Shaders()->color_shader.get();
  shader->Bind();
  shader->SetTransOffset(base::Vec2i());
  shader->SetViewportMatrix(GetSize());

  quad->Draw();

  cc->States()->blend->Pop();
  cc->States()->viewport->Pop();
  frame_buffer_->Unbind();

  NeedUpdate();
}

}  // namespace modules
