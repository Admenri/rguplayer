// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/font.h"

#include <array>
#include <fstream>
#include <map>
#include <string>

#include "base/exceptions/exception.h"
#include "content/public/utility.h"

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

  // Storage map: <FontID, Size> -> FontObject
  std::map<std::pair<int, int>, TTF_Font*> font_cache;
  std::vector<std::string> path_cache;
};

std::unique_ptr<DefaultFontState> g_default_font_state = nullptr;

bool FindFontInternal(const std::string& name, std::string* out_path) {
  for (auto& path : kFontLookupDirs) {
    std::string font_path = path + name;
    std::ifstream file(font_path);
    if (file.good()) {
      if (out_path)
        *out_path = font_path;

      return true;
    }
  }

  return false;
}

TTF_Font* LoadFontInternal(int font_id, int size) {
  auto& cache = g_default_font_state->font_cache;
  auto& paths = g_default_font_state->path_cache;
  TTF_Font* font = nullptr;

  if (size >= 6 && size <= 96 && font_id >= 0) {
    // Find in global cache
    bool loaded = false;
    while (!font) {
      auto it = cache.find({font_id, size});
      if (it != cache.end()) {
        font = it->second;
        return font;
      } else if (loaded)
        break;

      // Load new font
      const auto& font_path = paths.at(font_id);
      auto* font_ptr = TTF_OpenFont(font_path.c_str(), size);
      auto* font_dup_ptr = TTF_OpenFont(font_path.c_str(), size);
      if (font_ptr) {
        cache.emplace(std::pair{font_id, size}, font_ptr);
        loaded = true;
      }
    }
  }

  // Failed to load font
  return nullptr;
}

}  // namespace

void Font::InitStaticFont() {
  g_default_font_state = std::make_unique<DefaultFontState>();

  g_default_font_state->default_color = new Color(255.0, 255.0, 255.0, 255.0);
  g_default_font_state->default_out_color = new Color(0, 0, 0, 255.0);
}

void Font::DestroyStaticFont() {
  auto& cache = g_default_font_state->font_cache;
  for (auto& it : cache)
    TTF_CloseFont(it.second);

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

Font::Font() : Font(GetDefaultName(), GetDefaultSize()) {}

Font::Font(const std::vector<std::string>& name)
    : Font(name, GetDefaultSize()) {}

Font::Font(const std::vector<std::string>& name, int size)
    : name_(name),
      size_(size),
      bold_(GetDefaultBold()),
      italic_(GetDefaultItalic()),
      outline_(GetDefaultOutline()),
      shadow_(GetDefaultShadow()),
      color_(new Color(*GetDefaultColor())),
      out_color_(new Color(*GetDefaultOutColor())),
      font_id_(-1) {}

Font::Font(const Font& other)
    : name_(other.name_),
      size_(other.size_),
      bold_(other.bold_),
      italic_(other.italic_),
      outline_(other.outline_),
      shadow_(other.shadow_),
      color_(new Color(*other.color_)),
      out_color_(new Color(*other.out_color_)),
      font_id_(other.font_id_) {}

const Font& Font::operator=(const Font& other) {
  name_ = other.name_;
  size_ = other.size_;
  bold_ = other.bold_;
  italic_ = other.italic_;
  outline_ = other.outline_;
  shadow_ = other.shadow_;
  *color_ = *other.color_;
  *out_color_ = *other.out_color_;
  font_id_ = other.font_id_;
  return other;
}

bool Font::Existed(const std::string& name) {
  return FindFontInternal(name, nullptr);
}

void Font::SetName(const std::vector<std::string>& name) {
  if (name_ == name)
    return;
  name_ = name;
  font_id_ = -1;
}

std::vector<std::string> Font::GetName() const {
  return name_;
}

void Font::SetSize(int size) {
  if (size_ == size)
    return;
  size_ = size;
  font_id_ = -1;
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

TTF_Font* Font::AsSDLFont() {
  if (font_id_ < 0)
    MakeFontIDInternal();

  TTF_Font* font = LoadFontInternal(font_id_, size_);
  if (!font)
    return nullptr;

  int font_style = TTF_STYLE_NORMAL;
  if (bold_)
    font_style |= TTF_STYLE_BOLD;
  if (italic_)
    font_style |= TTF_STYLE_ITALIC;

  TTF_SetFontStyle(font, font_style);

  return font;
}

TTF_Font* Font::AsSDLFont(int id, int size) {
  TTF_Font* font = LoadFontInternal(id, size);
  return font;
}

void Font::MakeFontIDInternal() {
  auto& cache = g_default_font_state->path_cache;
  std::vector<std::string> load_names(name_);

  // Load default font
  load_names.push_back("Default.ttf");

  for (auto& name : load_names) {
    std::string font_find_path;
    if (!FindFontInternal(name, &font_find_path))
      continue;

    auto it = std::find(cache.begin(), cache.end(), name);
    int id = static_cast<int>(it - cache.begin());

    if (it == cache.end())
      cache.push_back(font_find_path);

    if (font_id_ < 0)
      font_id_ = id;
  }
}

}  // namespace content