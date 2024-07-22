// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "content/public/bitmap.h"

#include <array>

#include "base/exceptions/exception.h"
#include "components/filesystem/filesystem.h"
#include "content/config/core_config.h"
#include "content/public/font.h"
#include "content/worker/renderer_worker.h"
#include "renderer/quad/quad_drawable.h"

#include "SDL_image.h"

namespace content {

namespace {

uint16_t utf8_to_ucs2(const char* _input, const char** end_ptr) {
  const unsigned char* input = reinterpret_cast<const unsigned char*>(_input);
  *end_ptr = _input;

  if (input[0] == 0)
    return -1;

  if (input[0] < 0x80) {
    *end_ptr = _input + 1;

    return input[0];
  }

  if ((input[0] & 0xE0) == 0xE0) {
    if (input[1] == 0 || input[2] == 0)
      return -1;

    *end_ptr = _input + 3;

    return (input[0] & 0x0F) << 12 | (input[1] & 0x3F) << 6 | (input[2] & 0x3F);
  }

  if ((input[0] & 0xC0) == 0xC0) {
    if (input[1] == 0)
      return -1;

    *end_ptr = _input + 2;

    return (input[0] & 0x1F) << 6 | (input[1] & 0x3F);
  }

  return -1;
}

}  // namespace

Bitmap::Bitmap(scoped_refptr<Graphics> host, int width, int height)
    : GraphicElement(host),
      Disposable(host.get()),
      font_(new Font(host->font_manager())),
      surface_buffer_(nullptr) {
  if (width <= 0 || height <= 0) {
    throw base::Exception(base::Exception::ContentError,
                          "Invalid bitmap create size: (%dx%d)", width, height);
  }

  if (width > screen()->renderer()->max_texture_size() ||
      height > screen()->renderer()->max_texture_size()) {
    throw base::Exception(base::Exception::OpenGLError,
                          "Unable to create large bitmap: (%dx%d)", width,
                          height);
  }

  size_ = base::Vec2i(width, height);
  texture_ = screen()->AllocTexture(size_, true);
}

Bitmap::Bitmap(scoped_refptr<Graphics> host, const std::string& filename)
    : GraphicElement(host),
      Disposable(host.get()),
      font_(new Font(host->font_manager())),
      surface_buffer_(nullptr) {
  auto file_handler = base::BindRepeating(
      [](SDL_Surface** surf, SDL_IOStream* ops, const std::string& ext) {
        *surf = IMG_LoadTyped_IO(ops, SDL_TRUE, ext.c_str());

        return !!*surf;
      },
      &surface_buffer_);
  host->filesystem()->OpenRead(filename, file_handler);

  if (!surface_buffer_) {
    throw base::Exception(base::Exception::ContentError,
                          "Failed to load image: '%s': %s", filename.c_str(),
                          SDL_GetError());
  }

  if (surface_buffer_->w > screen()->renderer()->max_texture_size() ||
      surface_buffer_->h > screen()->renderer()->max_texture_size()) {
    throw base::Exception(base::Exception::OpenGLError,
                          "Unable to load large image: (%dx%d)",
                          surface_buffer_->w, surface_buffer_->h);
  }

  size_ = base::Vec2i(surface_buffer_->w, surface_buffer_->h);
  if (surface_buffer_->format != SDL_PIXELFORMAT_ABGR8888) {
    SDL_Surface* conv =
        SDL_ConvertSurface(surface_buffer_, SDL_PIXELFORMAT_ABGR8888);
    SDL_DestroySurface(surface_buffer_);
    surface_buffer_ = conv;
  }

  size_t img_size = size_.x * size_.y * 4;
  texture_ = screen()->AllocTexture(size_, false, GL_RGBA,
                                    surface_buffer_->pixels, img_size);
}

Bitmap::~Bitmap() {
  Dispose();
}

scoped_refptr<Bitmap> Bitmap::Clone() {
  CheckIsDisposed();

  scoped_refptr<Bitmap> new_bitmap = new Bitmap(screen(), size_.x, size_.y);
  new_bitmap->Blt(0, 0, this, size_);
  *new_bitmap->font_ = *font_;

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

  screen()->renderer()->PostTask(
      base::BindOnce(&Bitmap::StretchBltInternal, texture_, dest_rect,
                     src_bitmap->texture_, src_rect, opacity / 255.0f));

  NeedUpdateSurface();
}

void Bitmap::FillRect(const base::Rect& rect, scoped_refptr<Color> color) {
  CheckIsDisposed();

  if (rect.width <= 0 || rect.height <= 0)
    return;

  screen()->renderer()->PostTask(base::BindOnce(
      &Bitmap::FillRectInternal, texture_, rect, color->AsBase()));

  NeedUpdateSurface();
}

void Bitmap::GradientFillRect(const base::Rect& rect,
                              scoped_refptr<Color> color1,
                              scoped_refptr<Color> color2,
                              bool vertical) {
  CheckIsDisposed();

  if (rect.width <= 0 || rect.height <= 0)
    return;

  screen()->renderer()->PostTask(
      base::BindOnce(&Bitmap::GradientFillRectInternal, texture_, rect,
                     color1->AsBase(), color2->AsBase(), vertical));

  NeedUpdateSurface();
}

void Bitmap::Clear() {
  CheckIsDisposed();

  screen()->renderer()->PostTask(
      base::BindOnce(&Bitmap::FillRectInternal, texture_, size_, base::Vec4()));

  NeedUpdateSurface();
}

void Bitmap::ClearRect(const base::Rect& rect) {
  CheckIsDisposed();

  screen()->renderer()->PostTask(
      base::BindOnce(&Bitmap::FillRectInternal, texture_, rect, base::Vec4()));

  NeedUpdateSurface();
}

scoped_refptr<Color> Bitmap::GetPixel(int x, int y) {
  CheckIsDisposed();

  if (x < 0 || x >= size_.x || y < 0 || y >= size_.y)
    return nullptr;

  SurfaceRequired();
  auto* pixel_detail = SDL_GetPixelFormatDetails(surface_buffer_->format);
  int bpp = pixel_detail->bytes_per_pixel;
  uint8_t* pixel = static_cast<uint8_t*>(surface_buffer_->pixels) +
                   y * surface_buffer_->pitch + x * bpp;

  uint8_t color[4];
  SDL_GetRGBA(*reinterpret_cast<uint32_t*>(pixel), pixel_detail, nullptr,
              &color[0], &color[1], &color[2], &color[3]);

  return new Color(color[0], color[1], color[2], color[3]);
}

void Bitmap::SetPixel(int x, int y, scoped_refptr<Color> color) {
  CheckIsDisposed();

  if (x < 0 || x >= size_.x || y < 0 || y >= size_.y)
    return;

  auto data = color->AsNormal();
  auto* pixel_detail = SDL_GetPixelFormatDetails(surface_buffer_->format);
  if (surface_buffer_) {
    int bpp = pixel_detail->bytes_per_pixel;
    uint8_t* pixel = static_cast<uint8_t*>(surface_buffer_->pixels) +
                     y * surface_buffer_->pitch + x * bpp;
    *reinterpret_cast<uint32_t*>(pixel) =
        SDL_MapRGBA(pixel_detail, nullptr, static_cast<uint8_t>(data.x),
                    static_cast<uint8_t>(data.y), static_cast<uint8_t>(data.z),
                    static_cast<uint8_t>(data.w));
  }

  screen()->renderer()->PostTask(
      base::BindOnce(&Bitmap::SetPixelInternal, texture_, x, y, data));

  NeedUpdateSurface();
}

void Bitmap::HueChange(int hue) {
  CheckIsDisposed();

  if (hue % 360 == 0)
    return;

  screen()->renderer()->PostTask(
      base::BindOnce(&Bitmap::HueChangeInternal, texture_, hue));

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
                      TextAlign align) {
  CheckIsDisposed();

  font_->EnsureLoadFont();
  uint8_t fopacity;
  auto* surf = font_->RenderText(str, &fopacity);
  if (surf)
    screen()->renderer()->PostTask(base::BindOnce(
        &Bitmap::DrawTextInternal, texture_, surf, fopacity, rect, align));

  NeedUpdateSurface();
}

scoped_refptr<Rect> Bitmap::TextSize(const std::string& str) {
  CheckIsDisposed();

  TTF_Font* font = font_->AsSDLFont();
  std::string src_text = font_->FixupString(str);

  int w, h;
  TTF_SizeUTF8(font, src_text.c_str(), &w, &h);

  const char* end_char = nullptr;
  uint16_t ucs2 = utf8_to_ucs2(str.c_str(), &end_char);
  if (font_->GetItalic() && *end_char == '\0')
    TTF_GlyphMetrics(font, ucs2, 0, 0, 0, 0, &w);

  return new Rect(base::Rect(0, 0, w, h));
}

scoped_refptr<Font> Bitmap::GetFont() const {
  CheckIsDisposed();
  return font_;
}

void Bitmap::SetFont(scoped_refptr<Font> font) {
  CheckIsDisposed();
  *font_ = *font;
}

SDL_Surface* Bitmap::SurfaceRequired() {
  CheckIsDisposed();

  if (surface_buffer_)
    return surface_buffer_;
  surface_buffer_ =
      SDL_CreateSurface(size_.x, size_.y, SDL_PIXELFORMAT_ABGR8888);

  std::atomic_bool sync_fence = false;
  screen()->renderer()->PostTask(base::BindOnce(
      &Bitmap::GetSurfaceInternal, texture_, surface_buffer_, &sync_fence));

  while (!sync_fence)
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

  return surface_buffer_;
}

void Bitmap::UpdateSurface() {
  CheckIsDisposed();

  if (surface_buffer_ && surface_buffer_->pixels) {
    std::vector<uint8_t> update_data;
    size_t data_len = size_.x * size_.y * 4;
    update_data.assign(data_len, 0);
    memcpy(update_data.data(), surface_buffer_->pixels, data_len);

    screen()->renderer()->PostTask(base::BindOnce(
        &Bitmap::UpdateSurfaceInternal, texture_, std::move(update_data)));
  }

  NeedUpdateSurface();
}

void Bitmap::OnObjectDisposed() {
  // Dispose notify
  observers_.Notify();

  if (surface_buffer_) {
    SDL_DestroySurface(surface_buffer_);
    surface_buffer_ = nullptr;
  }

  // Destroy async
  screen()->FreeTexture(texture_);
}

void Bitmap::StretchBltInternal(renderer::TextureFrameBuffer* texture,
                                const base::Rect& dest_rect,
                                renderer::TextureFrameBuffer* src_bitmap,
                                const base::Rect& src_rect,
                                float opacity) {
  base::Vec2i& size = texture->size;
  base::Vec2i& src_size = src_bitmap->size;
  auto& dst_tex =
      renderer::GSM.EnsureCommonTFB(dest_rect.width, dest_rect.height);

  renderer::Blt::BeginDraw(dst_tex);
  renderer::Blt::TexSource(*texture);
  renderer::Blt::BltDraw(dest_rect, dest_rect.Size());

  /*
   * (texCoord - src_offset) * src_dst_factor
   */
  base::Vec4 offset_scale;
  offset_scale.x = static_cast<float>(src_rect.x) / src_size.x;
  offset_scale.y = static_cast<float>(src_rect.y) / src_size.y;
  offset_scale.z = (static_cast<float>(src_size.x) / src_rect.width) *
                   (static_cast<float>(dest_rect.width) / dst_tex.size.x);
  offset_scale.w = (static_cast<float>(src_size.y) / src_rect.height) *
                   (static_cast<float>(dest_rect.height) / dst_tex.size.y);

  auto& shader = renderer::GSM.shaders()->texblt;

  renderer::GSM.states.viewport.Push(size);
  renderer::GSM.states.blend.Push(false);

  renderer::FrameBuffer::Bind(texture->fbo);

  shader.Bind();
  shader.SetProjectionMatrix(size);
  shader.SetTransOffset(base::Vec2i());
  shader.SetSrcTexture(src_bitmap->tex);
  shader.SetTextureSize(src_bitmap->size);
  shader.SetDstTexture(dst_tex.tex);
  shader.SetOffsetScale(offset_scale);
  shader.SetOpacity(opacity);

  auto* quad = renderer::GSM.common_quad();
  quad->SetPositionRect(dest_rect);
  quad->SetTexCoordRect(src_rect);
  quad->Draw();

  renderer::GSM.states.blend.Pop();
  renderer::GSM.states.viewport.Pop();
}

void Bitmap::FillRectInternal(renderer::TextureFrameBuffer* texture,
                              const base::Rect& rect,
                              const base::Vec4& color) {
  renderer::FrameBuffer::Bind(texture->fbo);

  renderer::GSM.states.scissor.Push(true);
  renderer::GSM.states.scissor_rect.Push(rect);

  renderer::GSM.states.clear_color.Push(color);
  renderer::FrameBuffer::Clear();
  renderer::GSM.states.clear_color.Pop();

  renderer::GSM.states.scissor_rect.Pop();
  renderer::GSM.states.scissor.Pop();
}

void Bitmap::GradientFillRectInternal(renderer::TextureFrameBuffer* texture,
                                      const base::Rect& rect,
                                      const base::Vec4& color1,
                                      const base::Vec4& color2,
                                      bool vertical) {
  base::Vec2i& size = texture->size;
  renderer::FrameBuffer::Bind(texture->fbo);

  renderer::GSM.states.viewport.Push(size);
  renderer::GSM.states.blend.Push(false);

  auto& shader = renderer::GSM.shaders()->color;
  shader.Bind();
  shader.SetProjectionMatrix(size);
  shader.SetTransOffset(base::Vec2i());

  auto* quad = renderer::GSM.common_quad();
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

void Bitmap::SetPixelInternal(renderer::TextureFrameBuffer* texture,
                              int x,
                              int y,
                              const base::Vec4& color) {
  std::array<uint8_t, 4> pixel = {
      static_cast<uint8_t>(color.x), static_cast<uint8_t>(color.y),
      static_cast<uint8_t>(color.z), static_cast<uint8_t>(color.w)};

  renderer::Texture::Bind(texture->tex);
  renderer::Texture::TexSubImage2D(x, y, 1, 1, GL_RGBA, pixel.data());
}

void Bitmap::HueChangeInternal(renderer::TextureFrameBuffer* texture, int hue) {
  base::Vec2i& size = texture->size;
  auto& dst_tex = renderer::GSM.EnsureCommonTFB(size.x, size.y);

  while (hue < 0)
    hue += 359;
  hue %= 359;

  renderer::FrameBuffer::Bind(dst_tex.fbo);
  renderer::FrameBuffer::Clear();

  renderer::GSM.states.viewport.Push(size);
  auto& shader = renderer::GSM.shaders()->hue;
  shader.Bind();
  shader.SetProjectionMatrix(size);
  shader.SetTexture(texture->tex);
  shader.SetTextureSize(size);
  shader.SetTransOffset(base::Vec2i());
  shader.SetHueAdjustValue(static_cast<float>(hue) / 360.0f);

  auto* quad = renderer::GSM.common_quad();
  quad->SetTexCoordRect(base::Vec2(size));
  quad->SetPositionRect(base::Vec2(size));
  quad->Draw();
  renderer::GSM.states.viewport.Pop();

  renderer::Blt::BeginDraw(*texture);
  renderer::Blt::TexSource(dst_tex);
  renderer::Blt::BltDraw(size, size);
}

void Bitmap::DrawTextInternal(renderer::TextureFrameBuffer* texture,
                              SDL_Surface* txt_surf,
                              uint8_t fopacity,
                              const base::Rect& rect,
                              TextAlign align) {
  base::Vec2i& size = texture->size;
  if (!txt_surf)
    return;

  int align_x = rect.x, align_y = rect.y + (rect.height - txt_surf->h) / 2;

  switch (align) {
    default:
    case TextAlign::Left:
      break;
    case TextAlign::Center:
      align_x += (rect.width - txt_surf->w) / 2;
      break;
    case TextAlign::Right:
      align_x += rect.width - txt_surf->w;
      break;
  }

  float zoom_x = static_cast<float>(rect.width) / txt_surf->w;
  zoom_x = std::min(zoom_x, 1.0f);
  base::Rect pos(align_x, align_y, txt_surf->w * zoom_x, txt_surf->h);

  auto& common_frame_buffer =
      renderer::GSM.EnsureCommonTFB(pos.width, pos.height);
  base::Vec2i origin_size = common_frame_buffer.size;

  renderer::Blt::BeginDraw(common_frame_buffer);
  renderer::Blt::TexSource(*texture);
  renderer::Blt::BltDraw(pos, pos.Size());

  base::Vec2i rendered_text_size;
  auto& generic_tex = renderer::GSM.EnsureGenericTex(txt_surf->w, txt_surf->h,
                                                     rendered_text_size);

  renderer::Texture::Bind(generic_tex);
  renderer::Texture::TexSubImage2D(0, 0, txt_surf->w, txt_surf->h, GL_RGBA,
                                   txt_surf->pixels);

  base::Vec4 offset_scale(
      0, 0, static_cast<float>(rendered_text_size.x * zoom_x) / origin_size.x,
      static_cast<float>(rendered_text_size.y) / origin_size.y);

  base::Vec2 text_surf_size = base::Vec2i(txt_surf->w, txt_surf->h);
  SDL_DestroySurface(txt_surf);

  renderer::GSM.states.viewport.Push(size);
  renderer::GSM.states.blend.Push(false);

  renderer::FrameBuffer::Bind(texture->fbo);

  auto& shader = renderer::GSM.shaders()->texblt;
  shader.Bind();
  shader.SetProjectionMatrix(size);
  shader.SetTransOffset(base::Vec2i());
  shader.SetSrcTexture(generic_tex);
  shader.SetTextureSize(rendered_text_size);
  shader.SetDstTexture(common_frame_buffer.tex);
  shader.SetOffsetScale(offset_scale);
  shader.SetOpacity(fopacity / 255.0f);

  auto* quad = renderer::GSM.common_quad();
  quad->SetPositionRect(pos);
  quad->SetTexCoordRect(text_surf_size);
  quad->Draw();

  renderer::GSM.states.blend.Pop();
  renderer::GSM.states.viewport.Pop();
}

void Bitmap::GetSurfaceInternal(renderer::TextureFrameBuffer* texture,
                                SDL_Surface* output,
                                std::atomic_bool* fence) {
  base::Vec2i& size = texture->size;
  renderer::GSM.states.viewport.Push(size);
  renderer::FrameBuffer::Bind(texture->fbo);
  renderer::GL.ReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE,
                          output->pixels);
  renderer::GSM.states.viewport.Pop();

  // Activeate sync fence
  *fence = true;
}

void Bitmap::NeedUpdateSurface() {
  // For get pixel cache
  if (surface_buffer_) {
    SDL_DestroySurface(surface_buffer_);
    surface_buffer_ = nullptr;
  }

  observers_.Notify();
}

void Bitmap::UpdateSurfaceInternal(renderer::TextureFrameBuffer* texture,
                                   std::vector<uint8_t> data) {
  base::Vec2i& size = texture->size;
  renderer::Texture::Bind(texture->tex);
  renderer::Texture::TexImage2D(size.x, size.y, GL_RGBA, data.data());
}

}  // namespace content
