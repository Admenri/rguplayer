// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/math/math.h"

namespace base {

SDL_Rect Rect::ToSDLRect() const { return SDL_Rect{x, y, width, height}; }

RectF Rect::ToFloatRect() const {
  return RectF(static_cast<float>(x), static_cast<float>(y),
               static_cast<float>(width), static_cast<float>(height));
}

}  // namespace base
