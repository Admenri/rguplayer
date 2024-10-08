// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "content/public/bitmap.h"

#include <array>

#include "base/exception/exception.h"
#include "components/filesystem/filesystem.h"
#include "content/public/font.h"

#include "SDL3_image/SDL_image.h"

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

Bitmap::Bitmap(scoped_refptr<Graphics> host, const base::Vec2i& size)
    : GraphicsElement(host.get()),
      Disposable(host.get()),
      size_(size),
      read_buffer_(BGFX_INVALID_HANDLE),
      font_(new Font(host->font_manager())),
      surface_buffer_(nullptr) {
  if (size.x <= 0 || size.y <= 0) {
    throw base::Exception(base::Exception::ContentError,
                          "Invalid bitmap create size: (%dx%d)", size.x,
                          size.y);
  }

  uint32_t size_limit = bgfx::getCaps()->limits.maxTextureSize;
  if (size.x > size_limit || size.y > size_limit) {
    throw base::Exception(base::Exception::RendererError,
                          "Unable to create large bitmap: (%dx%d)", size.x,
                          size.y);
  }

  bgfx::TextureHandle texture_buffer = bgfx::createTexture2D(
      size_.x, size_.y, false, 1, bgfx::TextureFormat::RGBA8, BGFX_TEXTURE_RT);
  texture_ = bgfx::createFrameBuffer(1, &texture_buffer, true);
}

Bitmap::Bitmap(scoped_refptr<Graphics> host,
               filesystem::Filesystem* io,
               const std::string& filename)
    : GraphicsElement(host.get()),
      Disposable(host.get()),
      read_buffer_(BGFX_INVALID_HANDLE),
      font_(new Font(host->font_manager())),
      surface_buffer_(nullptr) {
  auto file_handler = base::BindRepeating(
      [](SDL_Surface** surf, SDL_IOStream* ops, const std::string& ext) {
        *surf = IMG_LoadTyped_IO(ops, true, ext.c_str());

        return !!*surf;
      },
      &surface_buffer_);

  io->OpenRead(filename, file_handler);

  if (!surface_buffer_) {
    throw base::Exception(base::Exception::ContentError,
                          "Failed to load image: '%s': %s", filename.c_str(),
                          SDL_GetError());
  }

  uint32_t size_limit = bgfx::getCaps()->limits.maxTextureSize;
  if (surface_buffer_->w > size_limit || surface_buffer_->h > size_limit) {
    throw base::Exception(base::Exception::RendererError,
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

  size_t img_size =
      static_cast<size_t>(size_.x) * static_cast<size_t>(size_.y) * 4;
  bgfx::TextureHandle texture_buffer = bgfx::createTexture2D(
      size_.x, size_.y, false, 1, bgfx::TextureFormat::RGBA8, BGFX_TEXTURE_RT);

  bgfx::updateTexture2D(texture_buffer, 0, 0, 0, 0, surface_buffer_->w,
                        surface_buffer_->h,
                        bgfx::copy(surface_buffer_->pixels, img_size));
  texture_ = bgfx::createFrameBuffer(1, &texture_buffer, true);

  SDL_DestroySurface(surface_buffer_);
  surface_buffer_ = nullptr;
}

Bitmap::~Bitmap() {
  Dispose();
}

scoped_refptr<Bitmap> Bitmap::Clone() {
  CheckIsDisposed();

  scoped_refptr<Bitmap> new_bitmap =
      new Bitmap(static_cast<Graphics*>(screen()), size_);
  new_bitmap->Blt(base::Vec2i(), this, size_);
  *new_bitmap->font_ = *font_;

  return new_bitmap;
}

void Bitmap::Blt(const base::Vec2i& pos,
                 scoped_refptr<Bitmap> src_bitmap,
                 const base::Rect& src_rect,
                 int opacity) {
  CheckIsDisposed();

  if (src_rect.width <= 0 || src_rect.height <= 0)
    return;

  base::Rect rect = src_rect;
  if (rect.x + rect.width > src_bitmap->size_.x)
    rect.width = src_bitmap->size_.x - rect.x;

  if (rect.y + rect.height > src_bitmap->size_.y)
    rect.height = src_bitmap->size_.y - rect.y;

  rect.width = std::max(0, rect.width);
  rect.height = std::max(0, rect.height);

  StretchBlt(base::Rect(pos, rect.Size()), src_bitmap, rect, opacity);
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
  if (!IsObjectValid(src_bitmap.get()))
    return;

  bgfx::Encoder* encoder = bgfx::begin();
  bgfx::ViewId render_view = 0;
  {
    renderer::Texture intermediate_texture;
    screen()->device()->GetGenericTexture(dest_rect.Size(),
                                          &intermediate_texture);

    auto src_texture = bgfx::getTexture(texture_);
    encoder->blit(render_view, intermediate_texture.handle, 0, 0, src_texture,
                  dest_rect.x, dest_rect.y, dest_rect.width, dest_rect.height);

    screen()->device()->BindRenderView(render_view, size_, texture_,
                                       std::nullopt);

    // (texCoord - src_offset) * src_dst_factor
    base::Vec2i& src_size = src_bitmap->size_;
    base::Vec4 offset_scale;
    offset_scale.x = static_cast<float>(src_rect.x) / src_size.x;
    offset_scale.y = static_cast<float>(src_rect.y) / src_size.y;
    offset_scale.z =
        (static_cast<float>(src_size.x) / src_rect.width) *
        (static_cast<float>(dest_rect.width) / intermediate_texture.size.x);
    offset_scale.w =
        (static_cast<float>(src_size.y) / src_rect.height) *
        (static_cast<float>(dest_rect.height) / intermediate_texture.size.y);

    auto& shader = screen()->device()->pipelines().texblt;

    base::Vec4 offset_size =
        base::MakeVec4(base::Vec2(), base::MakeInvert(src_size));
    encoder->setUniform(shader.OffsetTexSize(), &offset_size);
    encoder->setUniform(shader.OffsetScale(), &offset_scale);

    base::Vec4 vec_opacity;
    vec_opacity.x = opacity / 255.0f;
    encoder->setUniform(shader.Opacity(), &vec_opacity);

    auto dst_texture = bgfx::getTexture(src_bitmap->texture_);
    encoder->setTexture(0, shader.Texture(), dst_texture);
    encoder->setTexture(1, shader.DstTexture(), intermediate_texture.handle);

    auto* quad = screen()->device()->common_quad();
    quad->SetPosition(dest_rect);
    quad->SetTexcoord(src_rect);

    encoder->setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
    quad->Draw(encoder, shader.GetProgram(), render_view);

    bgfx::end(encoder);
  }
  bgfx::frame();

  NeedUpdateSurface();
}

void Bitmap::FillRect(const base::Rect& rect, scoped_refptr<Color> color) {
  CheckIsDisposed();

  if (rect.width <= 0 || rect.height <= 0)
    return;

  bgfx::Encoder* encoder = bgfx::begin();
  bgfx::ViewId render_view = 0;
  {
    screen()->device()->BindRenderView(render_view, size_, texture_,
                                       std::nullopt);

    auto& shader = screen()->device()->pipelines().color;
    auto* quad = screen()->device()->common_quad();

    quad->SetPosition(rect);
    quad->SetColor(color->AsBase());

    encoder->setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
    quad->Draw(encoder, shader.GetProgram(), render_view);

    bgfx::end(encoder);
  }
  bgfx::frame();

  NeedUpdateSurface();
}

void Bitmap::GradientFillRect(const base::Rect& rect,
                              scoped_refptr<Color> color1,
                              scoped_refptr<Color> color2,
                              bool vertical) {
  CheckIsDisposed();

  if (rect.width <= 0 || rect.height <= 0)
    return;

  bgfx::Encoder* encoder = bgfx::begin();
  bgfx::ViewId render_view = 0;
  {
    screen()->device()->BindRenderView(render_view, size_, texture_,
                                       std::nullopt);

    auto& shader = screen()->device()->pipelines().color;
    auto* quad = screen()->device()->common_quad();

    quad->SetPosition(rect);
    if (vertical) {
      quad->SetColor(color1->AsBase(), 0);
      quad->SetColor(color1->AsBase(), 1);
      quad->SetColor(color2->AsBase(), 2);
      quad->SetColor(color2->AsBase(), 3);
    } else {
      quad->SetColor(color1->AsBase(), 0);
      quad->SetColor(color2->AsBase(), 1);
      quad->SetColor(color2->AsBase(), 2);
      quad->SetColor(color1->AsBase(), 3);
    }

    encoder->setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
    quad->Draw(encoder, shader.GetProgram(), render_view);

    bgfx::end(encoder);
  }
  bgfx::frame();

  NeedUpdateSurface();
}

void Bitmap::Clear() {
  ClearRect(size_);
}

void Bitmap::ClearRect(const base::Rect& rect) {
  CheckIsDisposed();

  bgfx::Encoder* encoder = bgfx::begin();
  bgfx::ViewId render_view = 0;
  {
    screen()->device()->BindRenderView(render_view, size_, texture_,
                                       std::nullopt);

    auto& shader = screen()->device()->pipelines().color;
    auto* quad = screen()->device()->common_quad();

    quad->SetPosition(rect);
    quad->SetColor(base::Vec4());

    encoder->setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
    quad->Draw(encoder, shader.GetProgram(), render_view);

    bgfx::end(encoder);
  }
  bgfx::frame();

  NeedUpdateSurface();
}

scoped_refptr<Color> Bitmap::GetPixel(const base::Vec2i& pos) {
  CheckIsDisposed();

  if (pos.x < 0 || pos.x >= size_.x || pos.y < 0 || pos.y >= size_.y)
    return nullptr;

  SurfaceRequired();
  auto* pixel_detail = SDL_GetPixelFormatDetails(surface_buffer_->format);
  int bpp = pixel_detail->bytes_per_pixel;
  uint8_t* pixel = static_cast<uint8_t*>(surface_buffer_->pixels) +
                   static_cast<size_t>(pos.y) * surface_buffer_->pitch +
                   static_cast<size_t>(pos.x) * bpp;

  uint8_t color[4];
  SDL_GetRGBA(*reinterpret_cast<uint32_t*>(pixel), pixel_detail, nullptr,
              &color[0], &color[1], &color[2], &color[3]);

  return new Color(color[0], color[1], color[2], color[3]);
}

void Bitmap::SetPixel(const base::Vec2i& pos, scoped_refptr<Color> color) {
  CheckIsDisposed();

  if (pos.x < 0 || pos.x >= size_.x || pos.y < 0 || pos.y >= size_.y)
    return;

  auto src_texture = bgfx::getTexture(texture_);
  SDL_Color color_u8 = color->AsSDLColor();
  bgfx::updateTexture2D(src_texture, 0, 0, pos.x, pos.y, 1, 1,
                        bgfx::copy(&color_u8, sizeof(color_u8)));

  NeedUpdateSurface();
}

void Bitmap::HueChange(int hue) {
  CheckIsDisposed();

  if (hue % 360 == 0)
    return;

  while (hue < 0)
    hue += 359;
  hue %= 359;

  renderer::Texture intermediate;
  screen()->device()->GetGenericTexture(size_, &intermediate);

  bgfx::Encoder* encoder = bgfx::begin();
  bgfx::ViewId render_view = 0;
  {
    auto src_texture = bgfx::getTexture(texture_);
    encoder->blit(render_view, intermediate.handle, 0, 0, src_texture, 0, 0,
                  size_.x, size_.y);

    screen()->device()->BindRenderView(render_view, size_, texture_,
                                       std::nullopt);

    auto& shader = screen()->device()->pipelines().hue;
    auto* quad = screen()->device()->common_quad();

    base::Vec4 offset_size =
        base::MakeVec4(base::Vec2(), base::MakeInvert(intermediate.size));
    encoder->setUniform(shader.OffsetTexSize(), &offset_size);
    base::Vec4 hue_adjust(static_cast<float>(hue) / 360.0f, 0, 0, 0);
    encoder->setUniform(shader.HueAdjustValue(), &hue_adjust);
    encoder->setTexture(0, shader.Texture(), intermediate.handle);

    quad->SetPosition(base::Vec2(size_));
    quad->SetTexcoord(base::Vec2(size_));

    encoder->setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
    quad->Draw(encoder, shader.GetProgram(), render_view);

    bgfx::end(encoder);
  }
  bgfx::frame();

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
  auto* txt_surf = font_->RenderText(str, &fopacity);
  if (txt_surf) {
    bgfx::Encoder* encoder = bgfx::begin();
    bgfx::ViewId render_view = 0;
    {
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

      renderer::Texture dst_texture;
      screen()->device()->GetGenericTexture(pos.Size(), &dst_texture);

      auto src_texture = bgfx::getTexture(texture_);
      encoder->blit(render_view, dst_texture.handle, 0, 0, src_texture, pos.x,
                    pos.y, pos.width, pos.height);

      renderer::Framebuffer text_texture;
      screen()->device()->EnsureCommonFramebuffer(
          base::Vec2i(txt_surf->w, txt_surf->h), &text_texture);

      auto text_tex = bgfx::getTexture(text_texture.handle);
      bgfx::updateTexture2D(
          text_tex, 0, 0, 0, 0, txt_surf->w, txt_surf->h,
          bgfx::copy(txt_surf->pixels, txt_surf->pitch * txt_surf->h));

      base::Vec4 offset_scale(
          0, 0,
          static_cast<float>(text_texture.size.x * zoom_x) / dst_texture.size.x,
          static_cast<float>(text_texture.size.y) / dst_texture.size.y);

      base::Vec2 text_surf_size = base::Vec2i(txt_surf->w, txt_surf->h);
      SDL_DestroySurface(txt_surf);

      screen()->device()->BindRenderView(render_view, size_, texture_,
                                         std::nullopt);

      auto& shader = screen()->device()->pipelines().texblt;

      base::Vec4 offset_size =
          base::MakeVec4(base::Vec2(), base::MakeInvert(text_texture.size));
      encoder->setUniform(shader.OffsetTexSize(), &offset_size);
      encoder->setUniform(shader.OffsetScale(), &offset_scale);

      base::Vec4 vec_opacity;
      vec_opacity.x = fopacity / 255.0f;
      encoder->setUniform(shader.Opacity(), &vec_opacity);
      encoder->setTexture(0, shader.Texture(), text_tex);
      encoder->setTexture(1, shader.DstTexture(), dst_texture.handle);

      auto* quad = screen()->device()->common_quad();
      quad->SetPosition(pos);
      quad->SetTexcoord(text_surf_size);

      encoder->setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
      quad->Draw(encoder, shader.GetProgram(), render_view);

      bgfx::end(encoder);
    }
    bgfx::frame();
  }

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

  if (!bgfx::isValid(read_buffer_))
    read_buffer_ = bgfx::createTexture2D(
        size_.x, size_.y, false, 1, bgfx::TextureFormat::RGBA8,
        BGFX_TEXTURE_READ_BACK | BGFX_TEXTURE_BLIT_DST);

  bgfx::Encoder* encoder = bgfx::begin();
  bgfx::ViewId render_view = 0;
  auto src_texture = bgfx::getTexture(texture_);
  encoder->blit(render_view, read_buffer_, 0, 0, src_texture);
  bgfx::end(encoder);

  uint32_t frame_sync =
      bgfx::readTexture(read_buffer_, surface_buffer_->pixels);
  uint32_t current_frame = bgfx::frame();
  while (current_frame < frame_sync)
    current_frame = bgfx::frame();

  return surface_buffer_;
}

void Bitmap::UpdateSurface() {
  CheckIsDisposed();

  if (surface_buffer_ && surface_buffer_->pixels) {
    size_t data_len =
        static_cast<size_t>(size_.x) * static_cast<size_t>(size_.y) * 4;

    auto src_texture = bgfx::getTexture(texture_);
    bgfx::updateTexture2D(src_texture, 0, 0, 0, 0, size_.x, size_.y,
                          bgfx::copy(surface_buffer_->pixels, data_len));
  }

  NeedUpdateSurface();
}

void Bitmap::OnObjectDisposed() {
  observers_.Notify();

  if (surface_buffer_) {
    SDL_DestroySurface(surface_buffer_);
    surface_buffer_ = nullptr;
  }

  if (bgfx::isValid(texture_))
    bgfx::destroy(texture_);
  if (bgfx::isValid(read_buffer_))
    bgfx::destroy(read_buffer_);
}

void Bitmap::NeedUpdateSurface() {
  observers_.Notify();

  if (surface_buffer_) {
    SDL_DestroySurface(surface_buffer_);
    surface_buffer_ = nullptr;
  }
}

}  // namespace content
