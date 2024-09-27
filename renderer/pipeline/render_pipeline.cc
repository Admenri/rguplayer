// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "renderer/pipeline/render_pipeline.h"

#include "renderer/pipeline/shaders/shader_set_autogen.h"

namespace renderer {

RenderShaderBase::~RenderShaderBase() {
  if (bgfx::isValid(program_))
    bgfx::destroy(program_);
}

bgfx::ProgramHandle RenderShaderBase::GetProgram() {
  return program_;
}

RenderShaderBase::RenderShaderBase() : program_(BGFX_INVALID_HANDLE) {}

void RenderShaderBase::CompileProgram(const bgfx::EmbeddedShader* vert_shader,
                                      const std::string& vert_name,
                                      const bgfx::EmbeddedShader* frag_shader,
                                      const std::string& frag_name) {
  bgfx::RendererType::Enum renderer = bgfx::getRendererType();
  bgfx::ShaderHandle vert =
      bgfx::createEmbeddedShader(vert_shader, renderer, vert_name.c_str());
  bgfx::ShaderHandle frag =
      bgfx::createEmbeddedShader(frag_shader, renderer, frag_name.c_str());
  program_ = bgfx::createProgram(vert, frag, true);
}

BaseShader::BaseShader() {
  RenderShaderBase::CompileProgram(&k_base_vert, "base_vert", &k_base_frag,
                                   "base_frag");

  u_offsetTexSize_ =
      bgfx::createUniform("u_offsetTexSize", bgfx::UniformType::Vec4);
  u_texture_ = bgfx::createUniform("u_texture", bgfx::UniformType::Sampler);
}

BaseShader::~BaseShader() {
  bgfx::destroy(u_offsetTexSize_);
  bgfx::destroy(u_texture_);
}

TexbltShader::TexbltShader() {
  RenderShaderBase::CompileProgram(&k_base_vert, "base_vert", &k_texblt_frag,
                                   "texblt_frag");

  u_offsetTexSize_ =
      bgfx::createUniform("u_offsetTexSize", bgfx::UniformType::Vec4);
  u_texture_ = bgfx::createUniform("u_texture", bgfx::UniformType::Sampler);
  u_dstTexture_ =
      bgfx::createUniform("u_dstTexture", bgfx::UniformType::Sampler);
  u_offsetScale_ =
      bgfx::createUniform("u_offsetScale", bgfx::UniformType::Vec4);
  u_opacity_ = bgfx::createUniform("u_opacity", bgfx::UniformType::Vec4);
}

TexbltShader::~TexbltShader() {
  bgfx::destroy(u_offsetTexSize_);
  bgfx::destroy(u_texture_);
  bgfx::destroy(u_dstTexture_);
  bgfx::destroy(u_offsetScale_);
  bgfx::destroy(u_opacity_);
}

BaseColorShader::BaseColorShader() {
  RenderShaderBase::CompileProgram(&k_basecolor_vert, "basecolor_vert",
                                   &k_basecolor_frag, "basecolor_frag");

  u_offsetTexSize_ =
      bgfx::createUniform("u_offsetTexSize", bgfx::UniformType::Vec4);
}

BaseColorShader::~BaseColorShader() {
  bgfx::destroy(u_offsetTexSize_);
}

HueShader::HueShader() {
  RenderShaderBase::CompileProgram(&k_base_vert, "base_vert", &k_hue_frag,
                                   "hue_frag");

  u_offsetTexSize_ =
      bgfx::createUniform("u_offsetTexSize", bgfx::UniformType::Vec4);
  u_texture_ = bgfx::createUniform("u_texture", bgfx::UniformType::Sampler);
  u_hueAdjustValue_ =
      bgfx::createUniform("u_hueAdjustValue", bgfx::UniformType::Vec4);
}

HueShader::~HueShader() {
  bgfx::destroy(u_offsetTexSize_);
  bgfx::destroy(u_texture_);
  bgfx::destroy(u_hueAdjustValue_);
}

}  // namespace renderer
