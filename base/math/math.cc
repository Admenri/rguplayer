#include "base/math/math.h"

namespace base {

SDL_Rect Rect::ToSDLRect() { return SDL_Rect{x, y, width, height}; }

RectF Rect::ToFloatRect() {
  return RectF(static_cast<float>(x), static_cast<float>(y),
               static_cast<float>(width), static_cast<float>(height));
}

}  // namespace base
