// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/font.h"

namespace content {

namespace {

const int kOutlineSize = 1;

std::vector<std::string> g_defaultName = {"Default.ttf"};
int g_DefaultSize = 24;
bool g_DefaultBold = false;
bool g_DefaultItalic = false;
bool g_DefaultOutline = true;
bool g_DefaultShadow = false;
scoped_refptr<Color> g_DefaultColor = nullptr;
scoped_refptr<Color> g_DefaultOutColor = nullptr;

}  // namespace

void Font::InitStaticFont() {
  g_DefaultColor = new Color(255, 255, 255);
  g_DefaultOutColor = new Color(0, 0, 0);
}

void Font::SetDefaultName(const std::vector<std::string>& name) {
  g_defaultName = name;
}

std::vector<std::string> Font::GetDefaultName() {
  return g_defaultName;
}

void Font::SetDefaultSize(int size) {
  g_DefaultSize = size;
}

int Font::GetDefaultSize() {
  return g_DefaultSize;
}

void Font::SetDefaultBold(bool bold) {
  g_DefaultBold = bold;
}

bool Font::GetDefaultBold() {
  return g_DefaultBold;
}

void Font::SetDefaultItalic(bool italic) {
  g_DefaultItalic = italic;
}

bool Font::GetDefaultItalic() {
  return g_DefaultItalic;
}

void Font::SetDefaultShadow(bool shadow) {
  g_DefaultShadow = shadow;
}

bool Font::GetDefaultShadow() {
  return g_DefaultShadow;
}

void Font::SetDefaultOutline(bool outline) {
  g_DefaultOutline = outline;
}

bool Font::GetDefaultOutline() {
  return g_DefaultOutline;
}

void Font::SetDefaultColor(scoped_refptr<Color> color) {
  g_DefaultColor = color;
}

scoped_refptr<Color> Font::GetDefaultColor() {
  return g_DefaultColor;
}

void Font::SetDefaultOutColor(scoped_refptr<Color> color) {
  g_DefaultOutColor = color;
}

scoped_refptr<Color> Font::GetDefaultOutColor() {
  return g_DefaultOutColor;
}

Font::Font()
    : name_(g_defaultName),
      size_(g_DefaultSize),
      bold_(g_DefaultBold),
      italic_(g_DefaultItalic),
      outline_(g_DefaultOutline),
      shadow_(g_DefaultShadow),
      color_(g_DefaultColor),
      out_color_(g_DefaultOutColor) {}

Font::Font(const std::vector<std::string>& name)
    : name_(name),
      size_(g_DefaultSize),
      bold_(g_DefaultBold),
      italic_(g_DefaultItalic),
      outline_(g_DefaultOutline),
      shadow_(g_DefaultShadow),
      color_(g_DefaultColor),
      out_color_(g_DefaultOutColor) {}

Font::Font(const std::vector<std::string>& name, int size)
    : name_(name),
      size_(size),
      bold_(g_DefaultBold),
      italic_(g_DefaultItalic),
      outline_(g_DefaultOutline),
      shadow_(g_DefaultShadow),
      color_(g_DefaultColor),
      out_color_(g_DefaultOutColor) {}

Font::~Font() {
  ResetFontInternal();
}

bool Font::Existed(const std::string& name) {
  return false;
}

void Font::SetName(const std::vector<std::string>& name) {
  name_ = name;
}

std::vector<std::string> Font::GetName() const {
  return name_;
}

void Font::SetSize(int size) {
  size_ = size;

  ResetFontInternal();
}

int Font::GetSize() const {
  return size_;
}

void Font::SetBold(bool bold) {
  bold_ = bold;
}

bool Font::GetBold() const {
  return bold_;
}

void Font::SetItalic(bool italic) {
  italic_ = italic;
}

bool Font::GetItalic() const {
  return italic_;
}

void Font::SetShadow(bool shadow) {
  shadow_ = shadow;
}

bool Font::GetShadow() const {
  return shadow_;
}

void Font::SetOutline(bool outline) {
  outline_ = outline;
}

bool Font::GetOutline() const {
  return outline_;
}

void Font::SetColor(scoped_refptr<Color> color) {
  color_ = color;
}

scoped_refptr<Color> Font::GetColor() const {
  return color_;
}

void Font::SetOutColor(scoped_refptr<Color> color) {
  out_color_ = color;
}

scoped_refptr<Color> Font::GetOutColor() const {
  return out_color_;
}

TTF_Font* Font::AsSDLFont() {
  if (!sdl_font_)
    LoadFontInternal();

  int font_style = TTF_STYLE_NORMAL;

  if (bold_)
    font_style |= TTF_STYLE_BOLD;
  if (italic_)
    font_style |= TTF_STYLE_ITALIC;

  TTF_SetFontStyle(sdl_font_, font_style);

  return sdl_font_;
}

std::string Font::FixupString(const std::string& text) {
  std::string str(text);

  for (size_t i = 0; i < str.size(); ++i)
    if (str[i] == '\r' || str[i] == '\n')
      str[i] = ' ';

  return str;
}

SDL_Surface* Font::RenderText(const std::string& text, uint8_t* font_opacity) {
  auto ensure_format = [](SDL_Surface*& surf) {
    SDL_Surface* format_surf = surf;
    if (surf->format->format != SDL_PIXELFORMAT_ABGR8888) {
      format_surf = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_ABGR8888);

      SDL_DestroySurface(surf);
    }
    surf = format_surf;
  };

  std::string src_text = FixupString(text);
  if (src_text.empty())
    return nullptr;

  TTF_Font* font = AsSDLFont();
  SDL_Color font_color = color_->AsSDLColor();
  SDL_Color outline_color = out_color_->AsSDLColor();
  if (font_opacity) {
    *font_opacity = font_color.a;
  }

  font_color.a = 255;
  outline_color.a = 255;

  SDL_Surface* raw_surf =
      TTF_RenderUTF8_Blended(font, src_text.c_str(), font_color);
  ensure_format(raw_surf);

  if (shadow_) {
    SDL_Surface* shadow_surf = SDL_CreateSurface(
        raw_surf->w + 1, raw_surf->h + 1, SDL_PIXELFORMAT_ABGR8888);

    float font_red = font_color.r / 255.0f;
    float font_green = font_color.g / 255.0f;
    float font_blue = font_color.b / 255.0f;

    for (int y = 0; y < raw_surf->h + 1; ++y)
      for (int x = 0; x < raw_surf->w + 1; ++x) {
        uint32_t* out_pixel = reinterpret_cast<uint32_t*>(
                                  static_cast<uint8_t*>(shadow_surf->pixels) +
                                  y * shadow_surf->pitch) +
                              x;

        uint32_t src_pixel = 0, shadow_pixel = 0;
        uint32_t* sample_pixel = reinterpret_cast<uint32_t*>(
            static_cast<uint8_t*>(raw_surf->pixels) + y * raw_surf->pitch);

        if (x < raw_surf->w && y < raw_surf->h)
          src_pixel = sample_pixel[x];
        if (x > 0 && y > 0)
          shadow_pixel = sample_pixel[x - 1];

        shadow_pixel &= raw_surf->format->Amask;
        if (x == 0 || y == 0) {
          *out_pixel = src_pixel;
          continue;
        }

        if (x == raw_surf->w || y == raw_surf->h) {
          *out_pixel = shadow_pixel;
          continue;
        }

        uint8_t src_alpha, shadow_alpha;
        src_alpha =
            (src_pixel & raw_surf->format->Amask) >> raw_surf->format->Ashift;
        shadow_alpha = (shadow_pixel & raw_surf->format->Amask) >>
                       raw_surf->format->Ashift;

        if (src_alpha == 255 || shadow_alpha == 0) {
          *out_pixel = src_alpha;
          continue;
        }

        if (src_alpha == 0 && shadow_alpha == 0) {
          *out_pixel = 0;
          continue;
        }

        float fSrcA = src_alpha / 255.0f;
        float fShdA = shadow_alpha / 255.0f;

        float co2 = fShdA * (1.0f - fSrcA);
        float fa = fSrcA + co2;
        float co3 = fSrcA / fa;

        uint8_t r, g, b, a;
        r = static_cast<uint8_t>(std::clamp<float>(font_red * co3, 0, 1) *
                                 255.0f);
        g = static_cast<uint8_t>(std::clamp<float>(font_green * co3, 0, 1) *
                                 255.0f);
        b = static_cast<uint8_t>(std::clamp<float>(font_blue * co3, 0, 1) *
                                 255.0f);
        a = static_cast<uint8_t>(std::clamp<float>(fa, 0, 1) * 255.0f);

        *out_pixel = SDL_MapRGBA(raw_surf->format, r, g, b, a);
      }

    SDL_DestroySurface(raw_surf);
    raw_surf = shadow_surf;
  }

  if (outline_) {
    SDL_Surface* outline;
    TTF_SetFontOutline(font, kOutlineSize);
    outline = TTF_RenderUTF8_Blended(font, src_text.c_str(), outline_color);
    ensure_format(outline);
    SDL_Rect outRect = {kOutlineSize, kOutlineSize, raw_surf->w, raw_surf->h};
    SDL_SetSurfaceBlendMode(raw_surf, SDL_BLENDMODE_BLEND);
    SDL_BlitSurface(raw_surf, NULL, outline, &outRect);
    SDL_DestroySurface(raw_surf);
    raw_surf = outline;
    TTF_SetFontOutline(font, 0);
  }

  return raw_surf;
}

void Font::LoadFontInternal() {
  // TODO: Fonts cache manager
  sdl_font_ = TTF_OpenFont(name_.front().c_str(), size_);
}

void Font::ResetFontInternal() {
  TTF_CloseFont(sdl_font_);
  sdl_font_ = nullptr;
}

}  // namespace content