// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/gles2/gsm/gles_gsm.h"

namespace gpu {

GlobalStateManager GSM;

void GlobalStateManager::InitStates() {
  GL.Disable(GL_DEPTH_TEST);

  states.viewport.Init(base::Rect());
  states.program.Init(0);
  states.scissor.Init(false);
  states.scissor_rect.Init(base::Rect());
  states.blend.Init(true);
  states.blend_func.Init(GLBlendType::Normal);
  states.clear_color.Init(base::Vec4());

  shaders.reset(new GLShaderWare());

  quad_ibo.reset(new QuadIndexBuffer());
  quad_ibo->EnsureSize(1);
}

}  // namespace gpu
