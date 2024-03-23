// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_FONT_H_
#define CONTENT_PUBLIC_FONT_H_

#include "SDL_ttf.h"

#include <string>
#include <vector>

#include "base/memory/ref_counted.h"
#include "content/public/utility.h"

namespace content {

class Font : public base::RefCounted<Font> {
 public:
  static void InitStaticFont();
  static void DestroyStaticFont();

  static bool Existed(const std::string& name);
  static void SetDefaultName(const std::vector<std::string>& name);
  static std::vector<std::string> GetDefaultName();
  static void SetDefaultSize(int size);
  static int GetDefaultSize();
  static void SetDefaultBold(bool bold);
  static bool GetDefaultBold();
  static void SetDefaultItalic(bool italic);
  static bool GetDefaultItalic();
  static void SetDefaultShadow(bool shadow);
  static bool GetDefaultShadow();
  static void SetDefaultOutline(bool outline);
  static bool GetDefaultOutline();
  static void SetDefaultColor(scoped_refptr<Color> color);
  static scoped_refptr<Color> GetDefaultColor();
  static void SetDefaultOutColor(scoped_refptr<Color> color);
  static scoped_refptr<Color> GetDefaultOutColor();

  Font();
  Font(const std::vector<std::string>& name);
  Font(const std::vector<std::string>& name, int size);
  Font(const Font& other);

  const Font& operator=(const Font& other);

  void SetName(const std::vector<std::string>& name);
  std::vector<std::string> GetName() const;
  void SetSize(int size);
  int GetSize() const;
  void SetBold(bool bold);
  bool GetBold() const;
  void SetItalic(bool italic);
  bool GetItalic() const;
  void SetShadow(bool shadow);
  bool GetShadow() const;
  void SetOutline(bool outline);
  bool GetOutline() const;
  void SetColor(scoped_refptr<Color> color);
  scoped_refptr<Color> GetColor() const;
  void SetOutColor(scoped_refptr<Color> color);
  scoped_refptr<Color> GetOutColor() const;

  void EnsureLoadFont();
  TTF_Font* AsSDLFont();
  std::string FixupString(const std::string& text);
  SDL_Surface* RenderText(const std::string& text, uint8_t* font_opacity);

 private:
  void LoadFontInternal();
  void MakeFontIDInternal();
  static bool FindFontInternal(const std::string& name, std::string* out_path);

  std::vector<std::string> name_;
  int size_;
  bool bold_;
  bool italic_;
  bool outline_;
  bool shadow_;
  scoped_refptr<Color> color_;
  scoped_refptr<Color> out_color_;

  int font_id_;
  TTF_Font* font_;
};

}  // namespace content

#endif  // !CONTENT_PUBLIC_FONT_H_