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
  states.transform.Init(nullptr);

  shaders_ = std::make_unique<GLShaderWare>();
  quad_ibo_ = std::make_unique<QuadIndexBuffer>();
  quad_ibo_->EnsureSize(1);

  common_tfb_ = TextureFrameBuffer::Gen();
  TextureFrameBuffer::Alloc(common_tfb_, base::Vec2i(64, 64));
  TextureFrameBuffer::LinkFrameBuffer(common_tfb_);

  explicit_tfb_ = TextureFrameBuffer::Gen();
  TextureFrameBuffer::Alloc(explicit_tfb_, base::Vec2i(64, 64));
  TextureFrameBuffer::LinkFrameBuffer(explicit_tfb_);

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

  TextureFrameBuffer::Del(explicit_tfb_);
  TextureFrameBuffer::Del(common_tfb_);
  Texture::Del(generic_tex_);
}

TextureFrameBuffer& GlobalStateManager::EnsureCommonTFB(int width, int height) {
  if (common_tfb_.size.x >= width && common_tfb_.size.y >= height)
    return common_tfb_;

  width = std::max(width, common_tfb_.size.x);
  height = std::max(height, common_tfb_.size.y);

  TextureFrameBuffer::Alloc(common_tfb_, base::Vec2i(width, height));
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

TextureFrameBuffer& GlobalStateManager::ClampExplicitTFB(int width,
                                                         int height) {
  if (explicit_tfb_.size.x != width || explicit_tfb_.size.y != height)
    TextureFrameBuffer::Alloc(explicit_tfb_, base::Vec2i(width, height));

  return explicit_tfb_;
}

}  // namespace renderer