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
  Font(const std::vector<std::string>& name, int size = 24);
  ~Font();

  static bool Existed(const std::string& name);

  Font(const Font&) = delete;
  Font& operator=(const Font&) = delete;

 private:
  std::vector<std::string> name_;
  int size_ = 24;
  bool bold_ = false;
  bool italic_ = false;
  bool outline_ = true;
  bool shadow_ = false;
  scoped_refptr<Color> color_;
  scoped_refptr<Color> out_color_;

  TTF_Font* sdl_font_;
};

}  // namespace content

#endif  // !CONTENT_PUBLIC_FONT_H_