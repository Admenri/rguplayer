// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RENDERER_DRAWABLE_INPUT_LAYOUT_H_
#define RENDERER_DRAWABLE_INPUT_LAYOUT_H_

#include "base/math/rectangle.h"

#include "bgfx/bgfx.h"

namespace renderer {

class GeometryVertexLayout {
 public:
  using Data = struct {
    base::Vec4 position;
    base::Vec2 texcoord;
    base::Vec4 color;
  };

  static bgfx::VertexLayout GetLayout();

  static void SetPosition(Data* data, const base::RectF& pos);
  static void SetTexcoord(Data* data, const base::RectF& tex);
  static void SetColor(Data* data, const base::Vec4& color, int index = -1);
};

}  // namespace renderer

#endif  //! RENDERER_DRAWABLE_INPUT_LAYOUT_H_
