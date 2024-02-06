// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/font.h"

#include <map>

#include "base/exceptions/exception.h"
#include "content/public/utility.h"

#define OutlineSize 1

namespace content {

namespace {

struct DefaultFontState {
  std::vector<std::string> default_name = {"Default.ttf"};
  int default_size = 24;
  bool default_bold = false;
  bool default_italic = false;
  bool default_outline = true;
  bool default_shadow = false;
  scoped_refptr<Color> default_color = nullptr;
  scoped_refptr<Color> default_out_color = nullptr;
};

std::unique_ptr<DefaultFontState> g_default_font_state = nullptr;

std::map<std::string, scoped_refptr<FontCache>> g_font_cache;

}  // namespace

void Font::InitStaticFont() {
  g_default_font_state = std::make_unique<DefaultFontState>();

  g_default_font_state->default_color = new Color(255.0, 255.0, 255.0, 255.0);
  g_default_font_state->default_out_color = new Color(0, 0, 0, 255.0);
}

void Font::SetDefaultName(const std::vector<std::string>& name) {
  g_default_font_state->default_name = name;
}

std::vector<std::string> Font::GetDefaultName() {
  return g_default_font_state->default_name;
}

void Font::SetDefaultSize(int size) {
  g_default_font_state->default_size = size;
}

int Font::GetDefaultSize() {
  return g_default_font_state->default_size;
}

void Font::SetDefaultBold(bool bold) {
  g_default_font_state->default_bold = bold;
}

bool Font::GetDefaultBold() {
  return g_default_font_state->default_bold;
}

void Font::SetDefaultItalic(bool italic) {
  g_default_font_state->default_italic = italic;
}

bool Font::GetDefaultItalic() {
  return g_default_font_state->default_italic;
}

void Font::SetDefaultShadow(bool shadow) {
  g_default_font_state->default_shadow = shadow;
}

bool Font::GetDefaultShadow() {
  return g_default_font_state->default_shadow;
}

void Font::SetDefaultOutline(bool outline) {
  g_default_font_state->default_outline = outline;
}

bool Font::GetDefaultOutline() {
  return g_default_font_state->default_outline;
}

void Font::SetDefaultColor(scoped_refptr<Color> color) {
  *g_default_font_state->default_color = *color;
}

scoped_refptr<Color> Font::GetDefaultColor() {
  return g_default_font_state->default_color;
}

void Font::SetDefaultOutColor(scoped_refptr<Color> color) {
  *g_default_font_state->default_out_color = *color;
}

scoped_refptr<Color> Font::GetDefaultOutColor() {
  return g_default_font_state->default_out_color;
}

Font::Font()
    : name_(g_default_font_state->default_name),
      size_(g_default_font_state->default_size),
      bold_(g_default_font_state->default_bold),
      italic_(g_default_font_state->default_italic),
      outline_(g_default_font_state->default_outline),
      shadow_(g_default_font_state->default_shadow),
      color_(g_default_font_state->default_color),
      out_color_(g_default_font_state->default_out_color) {}

Font::Font(const std::vector<std::string>& name)
    : name_(name),
      size_(g_default_font_state->default_size),
      bold_(g_default_font_state->default_bold),
      italic_(g_default_font_state->default_italic),
      outline_(g_default_font_state->default_outline),
      shadow_(g_default_font_state->default_shadow),
      color_(g_default_font_state->default_color),
      out_color_(g_default_font_state->default_out_color) {}

Font::Font(const std::vector<std::string>& name, int size)
    : name_(name),
      size_(size),
      bold_(g_default_font_state->default_bold),
      italic_(g_default_font_state->default_italic),
      outline_(g_default_font_state->default_outline),
      shadow_(g_default_font_state->default_shadow),
      color_(g_default_font_state->default_color),
      out_color_(g_default_font_state->default_out_color) {}

Font::Font(const Font& other)
    : name_(other.name_),
      size_(other.size_),
      bold_(other.bold_),
      italic_(other.italic_),
      outline_(other.outline_),
      shadow_(other.shadow_),
      color_(other.color_),
      out_color_(other.out_color_),
      cache_(other.cache_) {}

const Font& Font::operator=(const Font& other) {
  name_ = other.name_;
  size_ = other.size_;
  bold_ = other.bold_;
  italic_ = other.italic_;
  outline_ = other.outline_;
  shadow_ = other.shadow_;
  color_ = other.color_;
  out_color_ = other.out_color_;
  cache_ = other.cache_;
  return other;
}

bool Font::Existed(const std::string& name) {
  // TODO: filesystem required
  auto* f = SDL_RWFromFile(name.c_str(), "r+");
  SDL_RWclose(f);

  return !!f;
}

void Font::SetName(const std::vector<std::string>& name) {
  if (name_ == name)
    return;

  name_ = name;
  cache_.reset();
}

std::vector<std::string> Font::GetName() const {
  return name_;
}

void Font::SetSize(int size) {
  if (size_ == size)
    return;

  size_ = size;
  cache_.reset();
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
  *color_ = *color;
}

scoped_refptr<Color> Font::GetColor() const {
  return color_;
}

void Font::SetOutColor(scoped_refptr<Color> color) {
  *out_color_ = *color;
}

scoped_refptr<Color> Font::GetOutColor() const {
  return out_color_;
}

void Font::EnsureLoadFont() {
  if (!cache_)
    LoadFontInternal();
}

TTF_Font* Font::AsSDLFont() {
  EnsureLoadFont();

  auto* font = cache_->font(size_);

  int font_style = TTF_STYLE_NORMAL;
  if (bold_)
    font_style |= TTF_STYLE_BOLD;
  if (italic_)
    font_style |= TTF_STYLE_ITALIC;

  TTF_SetFontStyle(font, font_style);

  return font;
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
    TTF_SetFontOutline(font, OutlineSize);
    outline = TTF_RenderUTF8_Blended(font, src_text.c_str(), outline_color);
    ensure_format(outline);
    SDL_Rect outRect = {OutlineSize, OutlineSize, raw_surf->w, raw_surf->h};
    SDL_SetSurfaceBlendMode(raw_surf, SDL_BLENDMODE_BLEND);
    SDL_BlitSurface(raw_surf, NULL, outline, &outRect);
    SDL_DestroySurface(raw_surf);
    raw_surf = outline;
    TTF_SetFontOutline(font, 0);
  }

  return raw_surf;
}

void Font::LoadFontInternal() {
  // Find in global cache
  for (auto& name : name_) {
    auto it = g_font_cache.find(name);
    if (it != g_font_cache.end()) {
      cache_ = it->second;
      return;
    }
  }

  // Load from file
  for (auto& name : name_) {
    auto* font_obj = TTF_OpenFont(name.c_str(), size_);
    if (font_obj) {
      cache_ = new FontCache(font_obj, size_);

      // Storage in global cache
      g_font_cache.insert(std::make_pair(name, cache_));

      return;
    }
  }

  // Failed to load font
  throw base::Exception::Exception(base::Exception::ContentError,
                                   "Failed to load Font.");
}

}  // namespace content