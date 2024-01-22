// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/font.h"

namespace content {

namespace {

const int kOutlineSize = 1;

std::vector<std::string> g_defaultName;
int g_DefaultSize = 24;
bool g_DefaultBold = false;
bool g_DefaultItalic = false;
bool g_DefaultOutline = true;
bool g_DefaultShadow = false;
scoped_refptr<Color> g_DefaultColor = nullptr;
scoped_refptr<Color> g_DefaultOutColor = nullptr;

}  // namespace

void Font::InitStaticFont() {
  g_DefaultColor = new Color();
  g_DefaultOutColor = new Color();
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

Font::~Font() {}

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
  int font_style = TTF_STYLE_NORMAL;

  if (bold_)
    font_style |= TTF_STYLE_BOLD;
  if (italic_)
    font_style |= TTF_STYLE_ITALIC;

  TTF_SetFontStyle(sdl_font_, font_style);

  return sdl_font_;
}

}  // namespace content