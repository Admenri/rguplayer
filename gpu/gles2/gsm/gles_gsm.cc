// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/gles2/gsm/gles_gsm.h"

namespace gpu {

GlobalStateManager GSM;

void GlobalStateManager::InitStates() {
  GL.Disable(GL_DEPTH_TEST);

  viewport.Set(base::Rect());
  program.Set(0);
  scissor.Set(false);
  scissor_rect.Set(base::Rect());
  blend.Set(true);
  blend_func.Set(GLBlendType::Normal);
}

}  // namespace gpu
