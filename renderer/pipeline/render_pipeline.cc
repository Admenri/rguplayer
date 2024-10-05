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

ViewportShader::ViewportShader() {
  RenderShaderBase::CompileProgram(&k_base_vert, "base_vert", &k_viewport_frag,
                                   "viewport_frag");

  u_offsetTexSize_ =
      bgfx::createUniform("u_offsetTexSize", bgfx::UniformType::Vec4);
  u_texture_ = bgfx::createUniform("u_texture", bgfx::UniformType::Sampler);
  u_color_ = bgfx::createUniform("u_color", bgfx::UniformType::Vec4);
  u_tone_ = bgfx::createUniform("u_tone", bgfx::UniformType::Vec4);
}

ViewportShader::~ViewportShader() {
  bgfx::destroy(u_offsetTexSize_);
  bgfx::destroy(u_texture_);
  bgfx::destroy(u_color_);
  bgfx::destroy(u_tone_);
}

SpriteShader::SpriteShader() {
  RenderShaderBase::CompileProgram(&k_transform_vert, "transform_vert",
                                   &k_sprite_frag, "sprite_frag");

  u_transformMat_ =
      bgfx::createUniform("u_transformMat", bgfx::UniformType::Mat4);
  u_offsetTexSize_ =
      bgfx::createUniform("u_offsetTexSize", bgfx::UniformType::Vec4);
  u_texture_ = bgfx::createUniform("u_texture", bgfx::UniformType::Sampler);
  u_color_ = bgfx::createUniform("u_color", bgfx::UniformType::Vec4);
  u_tone_ = bgfx::createUniform("u_tone", bgfx::UniformType::Vec4);
  u_drawInfo_ = bgfx::createUniform("u_drawInfo", bgfx::UniformType::Vec4);
}

SpriteShader::~SpriteShader() {
  bgfx::destroy(u_transformMat_);
  bgfx::destroy(u_offsetTexSize_);
  bgfx::destroy(u_texture_);
  bgfx::destroy(u_color_);
  bgfx::destroy(u_tone_);
  bgfx::destroy(u_drawInfo_);
}

AlphaSpriteShader::AlphaSpriteShader() {
  RenderShaderBase::CompileProgram(&k_transform_vert, "transform_vert",
                                   &k_alphasprite_frag, "alphasprite_frag");

  u_transformMat_ =
      bgfx::createUniform("u_transformMat", bgfx::UniformType::Mat4);
  u_offsetTexSize_ =
      bgfx::createUniform("u_offsetTexSize", bgfx::UniformType::Vec4);
  u_texture_ = bgfx::createUniform("u_texture", bgfx::UniformType::Sampler);
  u_opacity_ = bgfx::createUniform("u_opacity", bgfx::UniformType::Vec4);
}

AlphaSpriteShader::~AlphaSpriteShader() {
  bgfx::destroy(u_transformMat_);
  bgfx::destroy(u_offsetTexSize_);
  bgfx::destroy(u_texture_);
  bgfx::destroy(u_opacity_);
}

BaseSpriteShader::BaseSpriteShader() {
  RenderShaderBase::CompileProgram(&k_transform_vert, "transform_vert",
                                   &k_base_frag, "base_frag");

  u_transformMat_ =
      bgfx::createUniform("u_transformMat", bgfx::UniformType::Mat4);
  u_offsetTexSize_ =
      bgfx::createUniform("u_offsetTexSize", bgfx::UniformType::Vec4);
  u_texture_ = bgfx::createUniform("u_texture", bgfx::UniformType::Sampler);
}

BaseSpriteShader::~BaseSpriteShader() {
  bgfx::destroy(u_transformMat_);
  bgfx::destroy(u_offsetTexSize_);
  bgfx::destroy(u_texture_);
}

PlaneShader::PlaneShader() {
  RenderShaderBase::CompileProgram(&k_base_vert, "base_vert", &k_plane_frag,
                                   "plane_frag");

  u_offsetTexSize_ =
      bgfx::createUniform("u_offsetTexSize", bgfx::UniformType::Vec4);
  u_texture_ = bgfx::createUniform("u_texture", bgfx::UniformType::Sampler);
  u_color_ = bgfx::createUniform("u_color", bgfx::UniformType::Vec4);
  u_tone_ = bgfx::createUniform("u_tone", bgfx::UniformType::Vec4);
  u_opacity_ = bgfx::createUniform("u_opacity", bgfx::UniformType::Vec4);
}

PlaneShader::~PlaneShader() {
  bgfx::destroy(u_offsetTexSize_);
  bgfx::destroy(u_texture_);
  bgfx::destroy(u_color_);
  bgfx::destroy(u_tone_);
  bgfx::destroy(u_opacity_);
}

AlphaFlatShader::AlphaFlatShader() {
  RenderShaderBase::CompileProgram(&k_basecolor_vert, "basecolor_vert",
                                   &k_flatflash_frag, "flatflash_frag");

  u_alpha_ = bgfx::createUniform("u_alpha", bgfx::UniformType::Vec4);
}

AlphaFlatShader::~AlphaFlatShader() {
  bgfx::destroy(u_alpha_);
}

BaseAlphaShader::BaseAlphaShader() {
  RenderShaderBase::CompileProgram(&k_basealpha_vert, "basealpha_vert",
                                   &k_basealpha_frag, "basealpha_frag");

  u_offsetTexSize_ =
      bgfx::createUniform("u_offsetTexSize", bgfx::UniformType::Vec4);
  u_texture_ = bgfx::createUniform("u_texture", bgfx::UniformType::Sampler);
}

BaseAlphaShader::~BaseAlphaShader() {
  bgfx::destroy(u_offsetTexSize_);
  bgfx::destroy(u_texture_);
}

TilemapShader::TilemapShader() {
  RenderShaderBase::CompileProgram(&k_tilemap_vert, "tilemap_vert",
                                   &k_base_frag, "base_frag");

  u_offsetTexSize_ =
      bgfx::createUniform("u_offsetTexSize", bgfx::UniformType::Vec4);
  u_texture_ = bgfx::createUniform("u_texture", bgfx::UniformType::Sampler);
  u_tileSize_AnimateIndex_ =
      bgfx::createUniform("u_tileSize_AnimateIndex", bgfx::UniformType::Vec4);
}

TilemapShader::~TilemapShader() {
  bgfx::destroy(u_offsetTexSize_);
  bgfx::destroy(u_texture_);
  bgfx::destroy(u_tileSize_AnimateIndex_);
}

Tilemap2Shader::Tilemap2Shader() {
  RenderShaderBase::CompileProgram(&k_tilemap2_vert, "tilemap2_vert",
                                   &k_base_frag, "base_frag");

  u_offsetTexSize_ =
      bgfx::createUniform("u_offsetTexSize", bgfx::UniformType::Vec4);
  u_texture_ = bgfx::createUniform("u_texture", bgfx::UniformType::Sampler);
  u_tileSize_ = bgfx::createUniform("u_tileSize", bgfx::UniformType::Vec4);
  u_autotileAnimationOffset_ =
      bgfx::createUniform("u_autotileAnimationOffset", bgfx::UniformType::Vec4);
}

Tilemap2Shader::~Tilemap2Shader() {
  bgfx::destroy(u_offsetTexSize_);
  bgfx::destroy(u_texture_);
  bgfx::destroy(u_tileSize_);
  bgfx::destroy(u_autotileAnimationOffset_);
}

AlphaTransShader::AlphaTransShader() {
  RenderShaderBase::CompileProgram(&k_base_vert, "base_vert",
                                   &k_alphatrans_frag, "alphatrans_frag");

  u_offsetTexSize_ =
      bgfx::createUniform("u_offsetTexSize", bgfx::UniformType::Vec4);
  u_frozenTexture_ =
      bgfx::createUniform("u_frozenTexture", bgfx::UniformType::Sampler);
  u_currentTexture_ =
      bgfx::createUniform("u_currentTexture", bgfx::UniformType::Sampler);
  u_progress_ = bgfx::createUniform("u_progress", bgfx::UniformType::Vec4);
}

AlphaTransShader::~AlphaTransShader() {
  bgfx::destroy(u_offsetTexSize_);
  bgfx::destroy(u_frozenTexture_);
  bgfx::destroy(u_currentTexture_);
  bgfx::destroy(u_progress_);
}

VagueTransShader::VagueTransShader() {
  RenderShaderBase::CompileProgram(&k_base_vert, "base_vert",
                                   &k_vaguetrans_frag, "vaguetrans_frag");

  u_offsetTexSize_ =
      bgfx::createUniform("u_offsetTexSize", bgfx::UniformType::Vec4);
  u_frozenTexture_ =
      bgfx::createUniform("u_frozenTexture", bgfx::UniformType::Sampler);
  u_currentTexture_ =
      bgfx::createUniform("u_currentTexture", bgfx::UniformType::Sampler);
  u_transTexture_ =
      bgfx::createUniform("u_transTexture", bgfx::UniformType::Sampler);
  u_progressVague_ =
      bgfx::createUniform("u_progressVague", bgfx::UniformType::Vec4);
}

VagueTransShader::~VagueTransShader() {
  bgfx::destroy(u_offsetTexSize_);
  bgfx::destroy(u_frozenTexture_);
  bgfx::destroy(u_currentTexture_);
  bgfx::destroy(u_transTexture_);
  bgfx::destroy(u_progressVague_);
}

GeometryShader::GeometryShader() {
  RenderShaderBase::CompileProgram(&k_geometry_vert, "geometry_vert",
                                   &k_geometry_frag, "geometry_frag");

  u_offsetTexSize_ =
      bgfx::createUniform("u_offsetTexSize", bgfx::UniformType::Vec4);
  u_texture_ = bgfx::createUniform("u_texture", bgfx::UniformType::Sampler);
}

GeometryShader::~GeometryShader() {
  bgfx::destroy(u_offsetTexSize_);
  bgfx::destroy(u_texture_);
}

}  // namespace renderer
