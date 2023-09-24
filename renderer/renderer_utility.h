// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RENDERER_RENDERER_UTILITY_H_
#define RENDERER_RENDERER_UTILITY_H_

namespace renderer {

enum class BlendMode {
  Normal = 0,
  Default = Normal,

  Addition,
  Substraction,
};

}  // namespace renderer

#endif  // RENDERER_RENDERER_UTILITY_H_