// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/gles2/gsm/gles_gsm.h"

#include <algorithm>

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

  shaders = std::make_unique<GLShaderWare>();
  quad_ibo = std::make_unique<QuadIndexBuffer>();
  quad_ibo->EnsureSize(1);

  common_tfb = TextureFrameBuffer::Gen();

  common_quad = std::make_unique<QuadDrawable>();
}

void GlobalStateManager::QuitStates() {
  common_quad.reset();
  quad_ibo.reset();
  shaders.reset();

  TextureFrameBuffer::Del(common_tfb);
}

void GlobalStateManager::EnsureGenericTex(int width, int height) {
  if (common_tfb.width >= width && common_tfb.height >= height) return;

  width = std::clamp(width, common_tfb.width, std::numeric_limits<int>::max());
  height =
      std::clamp(height, common_tfb.height, std::numeric_limits<int>::max());

  TextureFrameBuffer::Alloc(common_tfb, width, height);
}

}  // namespace gpu
