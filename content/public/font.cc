// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/font.h"

#include <array>
#include <fstream>
#include <map>
#include <string>
#include <vector>

#include "base/exceptions/exception.h"
#include "content/public/utility.h"

#define OutlineSize 1

namespace content {

std::array<std::string, 1> kFontLookupDirs = {
    "Fonts/",
};

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

  // IO filesystem
  filesystem::Filesystem* io;

  // Storage map: <Name, Size> -> FontObject
  std::map<std::pair<std::string, int>, TTF_Font*> font_cache;

  // Memory font storage: <Name> -> Fonts memory data
  std::map<std::string, std::pair<int64_t, void*>> mem_fonts;
};

std::unique_ptr<DefaultFontState> g_default_font_state = nullptr;

void RenderShadowSurface(SDL_Surface*& in, const SDL_Color& color) {
  SDL_Surface* out =
      SDL_CreateSurface(in->w + 1, in->h + 1, in->format->format);
  float fr = color.r / 255.0f, fg = color.g / 255.0f, fb = color.b / 255.0f;

  for (int y = 0; y < in->h + 1; ++y) {
    for (int x = 0; x < in->w + 1; ++x) {
      uint32_t src = 0, shd = 0,
               *outP = (uint32_t*)((uint8_t*)out->pixels + y * out->pitch) + x;

      if (y < in->h && x < in->w)
        src = ((uint32_t*)((uint8_t*)in->pixels + y * in->pitch))[x];
      if (y > 0 && x > 0)
        shd = ((uint32_t*)((uint8_t*)in->pixels + (y - 1) * in->pitch))[x - 1] &
              in->format->Amask;

      if (x == 0 || y == 0 || src & in->format->Amask) {
        *outP = (x == in->w || y == in->h) ? shd : src;
        continue;
      }

      uint8_t srcA = (src & in->format->Amask) >> in->format->Ashift;
      float fSrcA = srcA / 255.0f,
            fShdA = ((shd & in->format->Amask) >> in->format->Ashift) / 255.0f;
      float fa = fSrcA + fShdA * (1.0f - fSrcA), co3 = fSrcA / fa;

      *outP = SDL_MapRGBA(
          in->format,
          static_cast<uint8_t>(std::clamp(fr * co3, 0.0f, 1.0f) * 255),
          static_cast<uint8_t>(std::clamp(fg * co3, 0.0f, 1.0f) * 255),
          static_cast<uint8_t>(std::clamp(fb * co3, 0.0f, 1.0f) * 255),
          static_cast<uint8_t>(std::clamp(fa, 0.0f, 1.0f) * 255));
    }
  }

  SDL_DestroySurface(in);
  in = out;
}

std::pair<int64_t, void*> ReadFontToMemory(SDL_IOStream* io) {
  int64_t fsize = SDL_GetIOSize(io);
  void* mem = malloc(fsize);
  SDL_ReadIO(io, mem, fsize);
  SDL_CloseIO(io);

  return std::make_pair(fsize, mem);
}

TTF_Font* ReadFontFromMemory(const std::string& path, int size) {
  auto it = g_default_font_state->mem_fonts.find(path);
  if (it != g_default_font_state->mem_fonts.end()) {
    SDL_IOStream* io = SDL_IOFromConstMem(it->second.second, it->second.first);
    return TTF_OpenFontIO(io, SDL_TRUE, size * 0.9f);
  }

  return nullptr;
}

}  // namespace

void Font::InitStaticFont(filesystem::Filesystem* io) {
  g_default_font_state = std::make_unique<DefaultFontState>();
  g_default_font_state->default_color = new Color(255.0, 255.0, 255.0, 255.0);
  g_default_font_state->default_out_color = new Color(0, 0, 0, 255.0);
  g_default_font_state->io = io;

  for (auto& dir : kFontLookupDirs) {
    auto files = io->EnumDir(dir);
    for (auto& file : files) {
      std::string filepath = dir + file;

      SDL_IOStream* font_ops = nullptr;
      try {
        font_ops = io->OpenReadRaw(filepath);
      } catch (...) {
        // Ignore font load error
      }

      g_default_font_state->mem_fonts.emplace(filepath,
                                              ReadFontToMemory(font_ops));
    }
  }
}

void* Font::GetDefaultFont(int64_t* font_size) {
  auto it = g_default_font_state->mem_fonts.find("Fonts/Default.ttf");
  if (it != g_default_font_state->mem_fonts.end()) {
    *font_size = it->second.first;
    return it->second.second;
  }

  return nullptr;
}

void Font::DestroyStaticFont() {
  for (auto& it : g_default_font_state->font_cache)
    TTF_CloseFont(it.second);

  for (auto& it : g_default_font_state->mem_fonts)
    free(it.second.second);

  g_default_font_state.reset();
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
    : name_(GetDefaultName()),
      size_(GetDefaultSize()),
      bold_(GetDefaultBold()),
      italic_(GetDefaultItalic()),
      outline_(GetDefaultOutline()),
      shadow_(GetDefaultShadow()),
      color_(new Color(*GetDefaultColor())),
      out_color_(new Color(*GetDefaultOutColor())),
      font_(nullptr) {}

Font::Font(const std::vector<std::string>& name)
    : name_(name),
      size_(GetDefaultSize()),
      bold_(GetDefaultBold()),
      italic_(GetDefaultItalic()),
      outline_(GetDefaultOutline()),
      shadow_(GetDefaultShadow()),
      color_(new Color(*GetDefaultColor())),
      out_color_(new Color(*GetDefaultOutColor())),
      font_(nullptr) {}

Font::Font(const std::vector<std::string>& name, int size)
    : name_(name),
      size_(size),
      bold_(GetDefaultBold()),
      italic_(GetDefaultItalic()),
      outline_(GetDefaultOutline()),
      shadow_(GetDefaultShadow()),
      color_(new Color(*GetDefaultColor())),
      out_color_(new Color(*GetDefaultOutColor())),
      font_(nullptr) {}

Font::Font(const Font& other)
    : name_(other.name_),
      size_(other.size_),
      bold_(other.bold_),
      italic_(other.italic_),
      outline_(other.outline_),
      shadow_(other.shadow_),
      color_(new Color(*other.color_)),
      out_color_(new Color(*other.out_color_)),
      font_(other.font_) {}

const Font& Font::operator=(const Font& other) {
  name_ = other.name_;
  size_ = other.size_;
  bold_ = other.bold_;
  italic_ = other.italic_;
  outline_ = other.outline_;
  shadow_ = other.shadow_;
  *color_ = *other.color_;
  *out_color_ = *other.out_color_;
  font_ = other.font_;
  return other;
}

bool Font::Existed(const std::string& name) {
  for (const auto& dir : kFontLookupDirs) {
    auto it = g_default_font_state->mem_fonts.find(dir + name);
    if (it != g_default_font_state->mem_fonts.end())
      return true;
  }

  return false;
}

void Font::SetName(const std::vector<std::string>& name) {
  if (name_ == name)
    return;

  name_ = name;
  font_ = nullptr;
}

std::vector<std::string> Font::GetName() const {
  return name_;
}

void Font::SetSize(int size) {
  if (size_ == size)
    return;

  size_ = size;
  font_ = nullptr;
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
  if (!font_)
    LoadFontInternal();
}

TTF_Font* Font::AsSDLFont() {
  EnsureLoadFont();

  int font_style = TTF_STYLE_NORMAL;
  if (bold_)
    font_style |= TTF_STYLE_BOLD;
  if (italic_)
    font_style |= TTF_STYLE_ITALIC;

  TTF_SetFontStyle(font_, font_style);

  return font_;
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
    SDL_Surface* format_surf = nullptr;
    if (surf->format->format != SDL_PIXELFORMAT_ABGR8888) {
      format_surf = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_ABGR8888);
      SDL_DestroySurface(surf);
      surf = format_surf;
    }
  };

  std::string src_text = FixupString(text);
  if (src_text.empty() || src_text == " ")
    return nullptr;

  TTF_Font* font = AsSDLFont();
  if (!font)
    return nullptr;

  SDL_Color font_color = color_->AsSDLColor();
  SDL_Color outline_color = out_color_->AsSDLColor();
  if (font_opacity)
    *font_opacity = font_color.a;

  font_color.a = 255;
  outline_color.a = 255;

  SDL_Surface* raw_surf =
      TTF_RenderUTF8_Blended(font, src_text.c_str(), font_color);
  if (!raw_surf)
    return nullptr;
  ensure_format(raw_surf);

  if (shadow_)
    RenderShadowSurface(raw_surf, font_color);

  if (outline_) {
    SDL_Surface* outline = nullptr;
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
  std::vector<std::string> load_names(name_);
  load_names.push_back("Default.ttf");

  auto& font_cache = g_default_font_state->font_cache;
  if (size_ >= 6 && size_ <= 96) {
    for (const auto& dir : kFontLookupDirs) {
      for (const auto& font_name : load_names) {
        std::string path(dir + font_name);

        // Find existed font object
        auto it = font_cache.find(std::make_pair(path, size_));
        if (it != font_cache.end()) {
          font_ = it->second;
          return;
        }

        // Load new font object
        TTF_Font* font_obj = ReadFontFromMemory(path, size_);
        if (font_obj) {
          // Storage new font obj
          font_ = font_obj;
          font_cache.emplace(std::make_pair(path, size_), font_);
          return;
        }
      }
    }
  }

  // Failed to load font
  std::string font_names;
  for (auto& it : name_)
    font_names = font_names + it + " ";
  throw base::Exception(base::Exception::ContentError,
                        "Failed to load Font: %s", font_names.c_str());
}

}  // namespace content