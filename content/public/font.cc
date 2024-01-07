// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/font.h"

namespace content {

Font::Font(const std::vector<std::string>& name, int size) {}

Font::~Font() {}

bool Font::Existed(const std::string& name) {
  return false;
}

}  // namespace content