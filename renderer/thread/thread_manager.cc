// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "renderer/thread/thread_manager.h"
#include "renderer/quad/quad_drawable.h"

namespace renderer {

thread_local GlobalStateManager GSM;

void GlobalStateManager::InitStates() {
  GL.Disable(GL_DEPTH_TEST);
  GL.GetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size_);
  GL.ActiveTexture(GL_TEXTURE0);

  states.viewport.Init(base::Rect());
  states.program.Init(0);
  states.scissor.Init(false);
  states.scissor_rect.Init(base::Rect());
  states.blend.Init(true);
  states.blend_func.Init(GLBlendType::Normal);
  states.clear_color.Init(base::Vec4());
  states.vertex_attrib.Init(0);

  shaders_ = std::make_unique<GLShaderWare>();
  quad_ibo_ = std::make_unique<QuadIndexBuffer>();
  quad_ibo_->EnsureSize(1);

  common_tfb_ = TextureFrameBuffer::Gen();
  TextureFrameBuffer::Alloc(common_tfb_, 64, 64);
  TextureFrameBuffer::LinkFrameBuffer(common_tfb_);
  FrameBuffer::Unbind();

  generic_tex_ = Texture::Gen();
  Texture::Bind(generic_tex_);
  Texture::SetFilter();
  Texture::SetWrap();
  Texture::TexImage2D(64, 64, GL_RGBA);
  generic_tex_size_ = base::Vec2i(64, 64);

  common_quad_ = std::make_unique<QuadDrawable>();
}

void GlobalStateManager::QuitStates() {
  common_quad_.reset();
  quad_ibo_.reset();
  shaders_.reset();

  TextureFrameBuffer::Del(common_tfb_);
  Texture::Del(generic_tex_);
}

TextureFrameBuffer& GlobalStateManager::EnsureCommonTFB(int width, int height) {
  if (common_tfb_.width >= width && common_tfb_.height >= height)
    return common_tfb_;

  width = std::max(width, common_tfb_.width);
  height = std::max(height, common_tfb_.height);

  TextureFrameBuffer::Alloc(common_tfb_, width, height);
  return common_tfb_;
}

GLID<Texture>& GlobalStateManager::EnsureGenericTex(int width,
                                                    int height,
                                                    base::Vec2i& out_size) {
  if (generic_tex_size_.x >= width && generic_tex_size_.y >= height) {
    out_size = generic_tex_size_;
    return generic_tex_;
  }

  width = std::max(width, generic_tex_size_.x);
  height = std::max(height, generic_tex_size_.y);

  generic_tex_size_ = base::Vec2i(width, height);
  Texture::Bind(generic_tex_);
  Texture::TexImage2D(width, height, GL_RGBA);

  out_size = generic_tex_size_;
  return generic_tex_;
}

}  // namespace renderer