// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "renderer/thread/thread_manager.h"
#include "renderer/quad/quad_drawable.h"

namespace renderer {

thread_local GlobalStateManager GSM;

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
  TextureFrameBuffer::Alloc(common_tfb, 64, 64);
  TextureFrameBuffer::LinkFrameBuffer(common_tfb);
  FrameBuffer::Unbind();

  common_quad = std::make_unique<QuadDrawable>();
}

void GlobalStateManager::QuitStates() {
  common_quad.reset();
  quad_ibo.reset();
  shaders.reset();

  TextureFrameBuffer::Del(common_tfb);
}

void GlobalStateManager::EnsureGenericTex(int width, int height) {
  if (common_tfb.width >= width && common_tfb.height >= height)
    return;

  width = std::max(width, common_tfb.width);
  height = std::max(height, common_tfb.height);

  TextureFrameBuffer::Alloc(common_tfb, width, height);
}

void GlobalStateManager::EnsureGenericTex(int width,
                                          int height,
                                          base::Vec2i& out_size) {
  if (common_tfb.width >= width && common_tfb.height >= height) {
    out_size.x = common_tfb.width;
    out_size.y = common_tfb.height;
    return;
  }

  width = std::max(width, common_tfb.width);
  height = std::max(height, common_tfb.height);

  TextureFrameBuffer::Alloc(common_tfb, width, height);
  out_size = base::Vec2i(width, height);
}

}  // namespace renderer