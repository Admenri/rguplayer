// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "renderer/shader/gles2_shaders.h"

#include "renderer/thread/thread_manager.h"

namespace renderer {

static char kGLESPrecisionDefine[] = "precision mediump float;";

namespace shader {

#include "renderer/shader/glsl/alphasprite.frag.xxd"
#include "renderer/shader/glsl/alphatrans.frag.xxd"
#include "renderer/shader/glsl/base.frag.xxd"
#include "renderer/shader/glsl/base.vert.xxd"
#include "renderer/shader/glsl/basealpha.frag.xxd"
#include "renderer/shader/glsl/basealpha.vert.xxd"
#include "renderer/shader/glsl/basecolor.frag.xxd"
#include "renderer/shader/glsl/basecolor.vert.xxd"
#include "renderer/shader/glsl/basesprite.frag.xxd"
#include "renderer/shader/glsl/flashtile.frag.xxd"
#include "renderer/shader/glsl/flat.frag.xxd"
#include "renderer/shader/glsl/geometry.frag.xxd"
#include "renderer/shader/glsl/geometry.vert.xxd"
#include "renderer/shader/glsl/hue.frag.xxd"
#include "renderer/shader/glsl/minimum.vert.xxd"
#include "renderer/shader/glsl/plane.frag.xxd"
#include "renderer/shader/glsl/spine.frag.xxd"
#include "renderer/shader/glsl/spine.vert.xxd"
#include "renderer/shader/glsl/sprite.frag.xxd"
#include "renderer/shader/glsl/texblt.frag.xxd"
#include "renderer/shader/glsl/tilemap.vert.xxd"
#include "renderer/shader/glsl/tilemap2.vert.xxd"
#include "renderer/shader/glsl/transform.vert.xxd"
#include "renderer/shader/glsl/vaguetrans.frag.xxd"
#include "renderer/shader/glsl/viewport.frag.xxd"
#include "renderer/shader/glsl/yuv.frag.xxd"

static inline std::string FromRawData(const uint8_t* raw_data,
                                      const uint32_t data_size) {
  return std::string(reinterpret_cast<const char*>(raw_data), data_size);
}

}  // namespace shader

GLES2Shader::GLES2Shader() {
  program_ = GL.CreateProgram();
  vertex_shader_ = GL.CreateShader(GL_VERTEX_SHADER);
  frag_shader_ = GL.CreateShader(GL_FRAGMENT_SHADER);
}

GLES2Shader::~GLES2Shader() {
  GL.DeleteShader(vertex_shader_);
  GL.DeleteShader(frag_shader_);
  GL.DeleteProgram(program_);
}

void GLES2Shader::Bind() {
  GSM.states.program.Set(program_);
}

void GLES2Shader::Unbind() {
  GL.ActiveTexture(GL_TEXTURE0);
  GSM.states.program.Set(0);
}

bool GLES2Shader::Setup(const std::string& vertex_shader,
                        const std::string& vertex_name,
                        const std::string& frag_shader,
                        const std::string& frag_name) {
  CompileShader(vertex_shader_, vertex_shader, vertex_name);
  CompileShader(frag_shader_, frag_shader, frag_name);

  GL.AttachShader(program(), vertex_shader_);
  GL.AttachShader(program(), frag_shader_);

  // Bind attribute
  if (BindAttribLocation()) {
    GL.BindAttribLocation(program(), AttribLocation::Position, "a_position");
    GL.BindAttribLocation(program(), AttribLocation::TexCoord, "a_texCoord");
    GL.BindAttribLocation(program(), AttribLocation::Color, "a_color");
  }

  // Before link bind
  GL.LinkProgram(program());

  GLint success;
  GL.GetProgramiv(program(), GL_LINK_STATUS, &success);
  if (!success) {
    GLint log_length;
    GL.GetProgramiv(program(), GL_INFO_LOG_LENGTH, &log_length);

    std::string log(log_length, '\0');
    GL.GetProgramInfoLog(program(), static_cast<GLsizei>(log.size()), 0,
                         log.data());

    LOG(ERROR) << "[GLSL] Program: " << log;
    return false;
  }

  return true;
}

bool GLES2Shader::CompileShader(GLuint glshader,
                                const std::string& shader_source,
                                const std::string& shader_name) {
  /*
     Vertex shader:
       0. Version Defines
       1. Common Header
       2. Shader body
   */

  std::vector<const GLchar*> shader_srcs;
  std::vector<GLint> shader_sizes;

  // Common header source
  if (GSM.enable_es_shaders()) {
    shader_srcs.push_back(
        reinterpret_cast<const GLchar*>(kGLESPrecisionDefine));
    shader_sizes.push_back(
        static_cast<GLint>(sizeof(kGLESPrecisionDefine) - 1));
  }

  // Setup shader source
  shader_srcs.push_back(reinterpret_cast<const GLchar*>(shader_source.c_str()));
  shader_sizes.push_back(static_cast<GLint>(shader_source.size()));

  // Setup shader program
  GL.ShaderSource(glshader, static_cast<GLsizei>(shader_srcs.size()),
                  shader_srcs.data(), shader_sizes.data());

  GL.CompileShader(glshader);
  GLint success;
  GL.GetShaderiv(glshader, GL_COMPILE_STATUS, &success);
  if (!success) {
    GLint log_length;
    GL.GetShaderiv(glshader, GL_INFO_LOG_LENGTH, &log_length);

    std::string log(log_length, '\0');
    GL.GetShaderInfoLog(glshader, static_cast<GLsizei>(log.size()), 0,
                        log.data());

    LOG(ERROR) << "[GLSL] " << shader_name << ": " << log;
    return false;
  }

  return true;
}

GLES2ShaderBase::GLES2ShaderBase() : u_projectionMat_(0) {}

bool GLES2ShaderBase::Setup(const std::string& vertex_shader,
                            const std::string& vertex_name,
                            const std::string& frag_shader,
                            const std::string& frag_name) {
  if (!GLES2Shader::Setup(vertex_shader, vertex_name, frag_shader, frag_name)) {
    LOG(ERROR) << "ShaderBase compile error.";
    return false;
  }

  u_projectionMat_ = GL.GetUniformLocation(program(), "u_projectionMat");
  return u_projectionMat_ >= 0;
}

void GLES2ShaderBase::SetTexture(GLint location, GLuint tex, uint16_t unit) {
  GL.ActiveTexture(GL_TEXTURE0 + unit);
  GL.BindTexture(GL_TEXTURE_2D, tex);
  GL.Uniform1i(location, unit);
  GL.ActiveTexture(GL_TEXTURE0);
}

void GLES2ShaderBase::SetProjectionMatrix(const base::Vec2i& size) {
  if (projection_cache_ == size)
    return;
  projection_cache_ = size;

  const float a = 2.f / size.x;
  const float b = 2.f / size.y;
  const float c = -2.f;
  GLfloat mat[16] = {a, 0, 0, 0, 0, b, 0, 0, 0, 0, c, 0, -1, -1, -1, 1};

  GL.UniformMatrix4fv(u_projectionMat_, 1, GL_FALSE, mat);
}

BaseShader::BaseShader() {
  GLES2ShaderBase::Setup(
      shader::FromRawData(shader::base_vert, shader::base_vert_len),
      "base_vert",
      shader::FromRawData(shader::base_frag, shader::base_frag_len),
      "base_frag");

  u_texSize_ = GL.GetUniformLocation(program(), "u_texSize");
  u_transOffset_ = GL.GetUniformLocation(program(), "u_transOffset");
  u_texture_ = GL.GetUniformLocation(program(), "u_texture");
}

void BaseShader::SetTextureSize(const base::Vec2& tex_size) {
  GL.Uniform2f(u_texSize_, 1.f / tex_size.x, 1.f / tex_size.y);
}

void BaseShader::SetTransOffset(const base::Vec2& offset) {
  GL.Uniform2f(u_transOffset_, offset.x, offset.y);
}

void BaseShader::SetTexture(GLID<Texture> tex) {
  GLES2ShaderBase::SetTexture(u_texture_, tex.gl, 1);
}

BaseAlphaShader::BaseAlphaShader() {
  GLES2ShaderBase::Setup(
      shader::FromRawData(shader::basealpha_vert, shader::basealpha_vert_len),
      "basealpha_vert",
      shader::FromRawData(shader::basealpha_frag, shader::basealpha_frag_len),
      "basealpha_frag");

  u_texSize_ = GL.GetUniformLocation(program(), "u_texSize");
  u_transOffset_ = GL.GetUniformLocation(program(), "u_transOffset");
  u_texture_ = GL.GetUniformLocation(program(), "u_texture");
}

void BaseAlphaShader::SetTextureSize(const base::Vec2& tex_size) {
  GL.Uniform2f(u_texSize_, 1.f / tex_size.x, 1.f / tex_size.y);
}

void BaseAlphaShader::SetTransOffset(const base::Vec2& offset) {
  GL.Uniform2f(u_transOffset_, offset.x, offset.y);
}

void BaseAlphaShader::SetTexture(GLID<Texture> tex) {
  GLES2ShaderBase::SetTexture(u_texture_, tex.gl, 1);
}

BaseSpriteShader::BaseSpriteShader() {
  GLES2ShaderBase::Setup(
      shader::FromRawData(shader::transform_vert, shader::transform_vert_len),
      "transform_vert",
      shader::FromRawData(shader::basesprite_frag, shader::basesprite_frag_len),
      "basesprite_frag");

  u_texSize_ = GL.GetUniformLocation(program(), "u_texSize");
  u_transformMat_ = GL.GetUniformLocation(program(), "u_transformMat");
  u_texture_ = GL.GetUniformLocation(program(), "u_texture");
}

void BaseSpriteShader::SetTextureSize(const base::Vec2& tex_size) {
  GL.Uniform2f(u_texSize_, 1.f / tex_size.x, 1.f / tex_size.y);
}

void BaseSpriteShader::SetTransformMatrix(const float* mat4) {
  GL.UniformMatrix4fv(u_transformMat_, 1, GL_FALSE, mat4);
}

void BaseSpriteShader::SetTexture(GLID<Texture> tex) {
  GLES2ShaderBase::SetTexture(u_texture_, tex.gl, 1);
}

AlphaSpriteShader::AlphaSpriteShader() {
  GLES2ShaderBase::Setup(
      shader::FromRawData(shader::transform_vert, shader::transform_vert_len),
      "transform_vert",
      shader::FromRawData(shader::alphasprite_frag,
                          shader::alphasprite_frag_len),
      "alphasprite_frag");

  u_texSize_ = GL.GetUniformLocation(program(), "u_texSize");
  u_transformMat_ = GL.GetUniformLocation(program(), "u_transformMat");
  u_texture_ = GL.GetUniformLocation(program(), "u_texture");

  u_opacity_ = GL.GetUniformLocation(program(), "u_opacity");
}

void AlphaSpriteShader::SetTextureSize(const base::Vec2& tex_size) {
  GL.Uniform2f(u_texSize_, 1.f / tex_size.x, 1.f / tex_size.y);
}

void AlphaSpriteShader::SetTransformMatrix(const float* mat4) {
  GL.UniformMatrix4fv(u_transformMat_, 1, GL_FALSE, mat4);
}

void AlphaSpriteShader::SetTexture(GLID<Texture> tex) {
  GLES2ShaderBase::SetTexture(u_texture_, tex.gl, 1);
}

void AlphaSpriteShader::SetOpacity(float opacity) {
  GL.Uniform1f(u_opacity_, opacity);
}

SpriteShader::SpriteShader() {
  GLES2ShaderBase::Setup(
      shader::FromRawData(shader::transform_vert, shader::transform_vert_len),
      "transform_vert",
      shader::FromRawData(shader::sprite_frag, shader::sprite_frag_len),
      "sprite_frag");

  u_texSize_ = GL.GetUniformLocation(program(), "u_texSize");
  u_transformMat_ = GL.GetUniformLocation(program(), "u_transformMat");
  u_texture_ = GL.GetUniformLocation(program(), "u_texture");

  u_opacity_ = GL.GetUniformLocation(program(), "u_opacity");
  u_color_ = GL.GetUniformLocation(program(), "u_color");
  u_tone_ = GL.GetUniformLocation(program(), "u_tone");

  u_bushDepth_ = GL.GetUniformLocation(program(), "u_bushDepth");
  u_bushOpacity_ = GL.GetUniformLocation(program(), "u_bushOpacity");
}

void SpriteShader::SetTextureSize(const base::Vec2& tex_size) {
  GL.Uniform2f(u_texSize_, 1.f / tex_size.x, 1.f / tex_size.y);
}

void SpriteShader::SetTransformMatrix(const float* mat4) {
  GL.UniformMatrix4fv(u_transformMat_, 1, GL_FALSE, mat4);
}

void SpriteShader::SetTexture(GLID<Texture> tex) {
  GLES2ShaderBase::SetTexture(u_texture_, tex.gl, 1);
}

void SpriteShader::SetOpacity(float opacity) {
  GL.Uniform1f(u_opacity_, opacity);
}

void SpriteShader::SetColor(const base::Vec4& color) {
  GL.Uniform4f(u_color_, color.x, color.y, color.z, color.w);
}

void SpriteShader::SetTone(const base::Vec4& tone) {
  GL.Uniform4f(u_tone_, tone.x, tone.y, tone.z, tone.w);
}

void SpriteShader::SetBushDepth(float bushDepth) {
  GL.Uniform1f(u_bushDepth_, bushDepth);
}

void SpriteShader::SetBushOpacity(float bushOpacity) {
  GL.Uniform1f(u_bushOpacity_, bushOpacity);
}

TexBltShader::TexBltShader() {
  GLES2ShaderBase::Setup(
      shader::FromRawData(shader::base_vert, shader::base_vert_len),
      "base_vert",
      shader::FromRawData(shader::texblt_frag, shader::texblt_frag_len),
      "texblt_frag");

  u_texSize_ = GL.GetUniformLocation(program(), "u_texSize");
  u_transOffset_ = GL.GetUniformLocation(program(), "u_transOffset");
  u_texture_ = GL.GetUniformLocation(program(), "u_texture");
  u_dst_texture_ = GL.GetUniformLocation(program(), "u_dst_texture");
  u_offset_scale_ = GL.GetUniformLocation(program(), "u_offset_scale");
  u_opacity_ = GL.GetUniformLocation(program(), "u_opacity");
}

void TexBltShader::SetTextureSize(const base::Vec2& tex_size) {
  GL.Uniform2f(u_texSize_, 1.f / tex_size.x, 1.f / tex_size.y);
}

void TexBltShader::SetTransOffset(const base::Vec2& offset) {
  GL.Uniform2f(u_transOffset_, offset.x, offset.y);
}

void TexBltShader::SetSrcTexture(GLID<Texture> tex) {
  GLES2ShaderBase::SetTexture(u_texture_, tex.gl, 1);
}

void TexBltShader::SetDstTexture(GLID<Texture> tex) {
  GLES2ShaderBase::SetTexture(u_dst_texture_, tex.gl, 2);
}

void TexBltShader::SetOffsetScale(const base::Vec4& offset_scale) {
  GL.Uniform4f(u_offset_scale_, offset_scale.x, offset_scale.y, offset_scale.z,
               offset_scale.w);
}

void TexBltShader::SetOpacity(float opacity) {
  GL.Uniform1f(u_opacity_, opacity);
}

ColorShader::ColorShader() {
  GLES2ShaderBase::Setup(
      shader::FromRawData(shader::basecolor_vert, shader::basecolor_vert_len),
      "basecolor_vert",
      shader::FromRawData(shader::basecolor_frag, shader::basecolor_frag_len),
      "basecolor_frag");

  u_transOffset_ = GL.GetUniformLocation(program(), "u_transOffset");
}

void ColorShader::SetTransOffset(const base::Vec2& offset) {
  GL.Uniform2f(u_transOffset_, offset.x, offset.y);
}

PlaneShader::PlaneShader() {
  GLES2ShaderBase::Setup(
      shader::FromRawData(shader::base_vert, shader::base_vert_len),
      "base_vert",
      shader::FromRawData(shader::plane_frag, shader::plane_frag_len),
      "plane_frag");

  u_transOffset_ = GL.GetUniformLocation(program(), "u_transOffset");
  u_texture_ = GL.GetUniformLocation(program(), "u_texture");
  u_texSize_ = GL.GetUniformLocation(program(), "u_texSize");

  u_opacity_ = GL.GetUniformLocation(program(), "u_opacity");
  u_color_ = GL.GetUniformLocation(program(), "u_color");
  u_tone_ = GL.GetUniformLocation(program(), "u_tone");
}

void PlaneShader::SetTransOffset(const base::Vec2& offset) {
  GL.Uniform2f(u_transOffset_, offset.x, offset.y);
}

void PlaneShader::SetTexture(GLID<Texture> tex) {
  GLES2ShaderBase::SetTexture(u_texture_, tex.gl, 1);
}

void PlaneShader::SetTextureSize(const base::Vec2& tex_size) {
  GL.Uniform2f(u_texSize_, 1.f / tex_size.x, 1.f / tex_size.y);
}

void PlaneShader::SetOpacity(float opacity) {
  GL.Uniform1f(u_opacity_, opacity);
}

void PlaneShader::SetColor(const base::Vec4& color) {
  GL.Uniform4f(u_color_, color.x, color.y, color.z, color.w);
}

void PlaneShader::SetTone(const base::Vec4& tone) {
  GL.Uniform4f(u_tone_, tone.x, tone.y, tone.z, tone.w);
}

ViewportShader::ViewportShader() {
  GLES2ShaderBase::Setup(
      shader::FromRawData(shader::base_vert, shader::base_vert_len),
      "base_vert",
      shader::FromRawData(shader::viewport_frag, shader::viewport_frag_len),
      "gray_frag");

  u_texSize_ = GL.GetUniformLocation(program(), "u_texSize");
  u_transOffset_ = GL.GetUniformLocation(program(), "u_transOffset");
  u_texture_ = GL.GetUniformLocation(program(), "u_texture");
  u_color_ = GL.GetUniformLocation(program(), "u_color");
  u_tone_ = GL.GetUniformLocation(program(), "u_tone");
}

void ViewportShader::SetTextureSize(const base::Vec2& tex_size) {
  GL.Uniform2f(u_texSize_, 1.f / tex_size.x, 1.f / tex_size.y);
}

void ViewportShader::SetTransOffset(const base::Vec2& offset) {
  GL.Uniform2f(u_transOffset_, offset.x, offset.y);
}

void ViewportShader::SetTexture(GLID<Texture> tex) {
  GLES2ShaderBase::SetTexture(u_texture_, tex.gl, 1);
}

void ViewportShader::SetColor(const base::Vec4& color) {
  GL.Uniform4f(u_color_, color.x, color.y, color.z, color.w);
}

void ViewportShader::SetTone(const base::Vec4& tone) {
  GL.Uniform4f(u_tone_, tone.x, tone.y, tone.z, tone.w);
}

FlatShader::FlatShader() {
  GLES2ShaderBase::Setup(
      shader::FromRawData(shader::minimum_vert, shader::minimum_vert_len),
      "minimum_vert",
      shader::FromRawData(shader::flat_frag, shader::flat_frag_len),
      "flat_frag");

  u_color_ = GL.GetUniformLocation(program(), "u_color");
}

void FlatShader::SetColor(const base::Vec4& color) {
  GL.Uniform4f(u_color_, color.x, color.y, color.z, color.w);
}

AlphaTransShader::AlphaTransShader() {
  GLES2ShaderBase::Setup(
      shader::FromRawData(shader::base_vert, shader::base_vert_len),
      "base_vert",
      shader::FromRawData(shader::alphatrans_frag, shader::alphatrans_frag_len),
      "alphatrans_frag");

  u_texSize_ = GL.GetUniformLocation(program(), "u_texSize");
  u_transOffset_ = GL.GetUniformLocation(program(), "u_transOffset");
  u_frozen_texture_ = GL.GetUniformLocation(program(), "u_frozenTexture");
  u_current_texture_ = GL.GetUniformLocation(program(), "u_currentTexture");
  u_progress_ = GL.GetUniformLocation(program(), "u_progress");
}

void AlphaTransShader::SetTextureSize(const base::Vec2& tex_size) {
  GL.Uniform2f(u_texSize_, 1.f / tex_size.x, 1.f / tex_size.y);
}

void AlphaTransShader::SetTransOffset(const base::Vec2& offset) {
  GL.Uniform2f(u_transOffset_, offset.x, offset.y);
}

void AlphaTransShader::SetFrozenTexture(GLID<Texture> tex) {
  SetTexture(u_frozen_texture_, tex.gl, 1);
}

void AlphaTransShader::SetCurrentTexture(GLID<Texture> tex) {
  SetTexture(u_current_texture_, tex.gl, 2);
}

void AlphaTransShader::SetProgress(float progress) {
  GL.Uniform1f(u_progress_, progress);
}

VagueTransShader::VagueTransShader() {
  GLES2ShaderBase::Setup(
      shader::FromRawData(shader::base_vert, shader::base_vert_len),
      "base_vert",
      shader::FromRawData(shader::vaguetrans_frag, shader::vaguetrans_frag_len),
      "vaguetrans_frag");

  u_texSize_ = GL.GetUniformLocation(program(), "u_texSize");
  u_transOffset_ = GL.GetUniformLocation(program(), "u_transOffset");
  u_frozen_texture_ = GL.GetUniformLocation(program(), "u_frozenTexture");
  u_current_texture_ = GL.GetUniformLocation(program(), "u_currentTexture");
  u_trans_texture_ = GL.GetUniformLocation(program(), "u_transTexture");
  u_progress_ = GL.GetUniformLocation(program(), "u_progress");
  u_vague_ = GL.GetUniformLocation(program(), "u_vague");
}

void VagueTransShader::SetTextureSize(const base::Vec2& tex_size) {
  GL.Uniform2f(u_texSize_, 1.f / tex_size.x, 1.f / tex_size.y);
}

void VagueTransShader::SetTransOffset(const base::Vec2& offset) {
  GL.Uniform2f(u_transOffset_, offset.x, offset.y);
}

void VagueTransShader::SetFrozenTexture(GLID<Texture> tex) {
  SetTexture(u_frozen_texture_, tex.gl, 1);
}

void VagueTransShader::SetCurrentTexture(GLID<Texture> tex) {
  SetTexture(u_current_texture_, tex.gl, 2);
}

void VagueTransShader::SetTransTexture(GLID<Texture> tex) {
  SetTexture(u_trans_texture_, tex.gl, 3);
}

void VagueTransShader::SetProgress(float progress) {
  GL.Uniform1f(u_progress_, progress);
}

void VagueTransShader::SetVague(float vague) {
  GL.Uniform1f(u_vague_, vague);
}

FlashTileShader::FlashTileShader() {
  GLES2ShaderBase::Setup(
      shader::FromRawData(shader::basecolor_vert, shader::basecolor_vert_len),
      "basecolor_vert",
      shader::FromRawData(shader::flashtile_frag, shader::flashtile_frag_len),
      "flashtile_frag");

  u_transOffset_ = GL.GetUniformLocation(program(), "u_transOffset");
  u_alpha_ = GL.GetUniformLocation(program(), "u_alpha");
}

void FlashTileShader::SetTransOffset(const base::Vec2& offset) {
  GL.Uniform2f(u_transOffset_, offset.x, offset.y);
}

void FlashTileShader::SetAlpha(float alpha) {
  GL.Uniform1f(u_alpha_, alpha);
}

Tilemap2Shader::Tilemap2Shader() {
  GLES2ShaderBase::Setup(
      shader::FromRawData(shader::tilemap2_vert, shader::tilemap2_vert_len),
      "tilemap2_vert",
      shader::FromRawData(shader::base_frag, shader::base_frag_len),
      "base_frag");

  u_texSize_ = GL.GetUniformLocation(program(), "u_texSize");
  u_transOffset_ = GL.GetUniformLocation(program(), "u_transOffset");
  u_texture_ = GL.GetUniformLocation(program(), "u_texture");

  u_autotileAnimationOffset_ =
      GL.GetUniformLocation(program(), "u_autotileAnimationOffset");
  u_tileSize_ = GL.GetUniformLocation(program(), "u_tileSize");
}

void Tilemap2Shader::SetTextureSize(const base::Vec2& tex_size) {
  GL.Uniform2f(u_texSize_, 1.f / tex_size.x, 1.f / tex_size.y);
}

void Tilemap2Shader::SetTransOffset(const base::Vec2& offset) {
  GL.Uniform2f(u_transOffset_, offset.x, offset.y);
}

void Tilemap2Shader::SetTexture(GLID<Texture> tex) {
  GLES2ShaderBase::SetTexture(u_texture_, tex.gl, 1);
}

void Tilemap2Shader::SetAnimationOffset(const base::Vec2& offset) {
  GL.Uniform2f(u_autotileAnimationOffset_, offset.x, offset.y);
}

void Tilemap2Shader::SetTileSize(float size) {
  GL.Uniform1f(u_tileSize_, size);
}

HueShader::HueShader() {
  GLES2ShaderBase::Setup(
      shader::FromRawData(shader::base_vert, shader::base_vert_len),
      "base_vert", shader::FromRawData(shader::hue_frag, shader::hue_frag_len),
      "hue_frag");

  u_texSize_ = GL.GetUniformLocation(program(), "u_texSize");
  u_transOffset_ = GL.GetUniformLocation(program(), "u_transOffset");
  u_texture_ = GL.GetUniformLocation(program(), "u_texture");
  u_hueAdjustValue_ = GL.GetUniformLocation(program(), "u_hueAdjustValue");
}

void HueShader::SetTextureSize(const base::Vec2& tex_size) {
  GL.Uniform2f(u_texSize_, 1.f / tex_size.x, 1.f / tex_size.y);
}

void HueShader::SetTransOffset(const base::Vec2& offset) {
  GL.Uniform2f(u_transOffset_, offset.x, offset.y);
}

void HueShader::SetTexture(GLID<Texture> tex) {
  GLES2ShaderBase::SetTexture(u_texture_, tex.gl, 1);
}

void HueShader::SetHueAdjustValue(float value) {
  GL.Uniform1f(u_hueAdjustValue_, value);
}

TilemapShader::TilemapShader() {
  GLES2ShaderBase::Setup(
      shader::FromRawData(shader::tilemap_vert, shader::tilemap_vert_len),
      "tilemap_vert",
      shader::FromRawData(shader::base_frag, shader::base_frag_len),
      "base_frag");

  u_texSize_ = GL.GetUniformLocation(program(), "u_texSize");
  u_transOffset_ = GL.GetUniformLocation(program(), "u_transOffset");
  u_texture_ = GL.GetUniformLocation(program(), "u_texture");
  u_tileSize_ = GL.GetUniformLocation(program(), "u_tileSize");
  u_animateIndex_ = GL.GetUniformLocation(program(), "u_animateIndex");
}

void TilemapShader::SetTextureSize(const base::Vec2& tex_size) {
  GL.Uniform2f(u_texSize_, 1.f / tex_size.x, 1.f / tex_size.y);
}

void TilemapShader::SetTransOffset(const base::Vec2& offset) {
  GL.Uniform2f(u_transOffset_, offset.x, offset.y);
}

void TilemapShader::SetTexture(GLID<Texture> tex) {
  GLES2ShaderBase::SetTexture(u_texture_, tex.gl, 1);
}

void TilemapShader::SetTileSize(float size) {
  GL.Uniform1f(u_tileSize_, size);
}

void TilemapShader::SetAnimateIndex(float index) {
  GL.Uniform1f(u_animateIndex_, index);
}

GeometryShader::GeometryShader() {
  GLES2ShaderBase::Setup(
      shader::FromRawData(shader::geometry_vert, shader::geometry_vert_len),
      "tilemap_vert",
      shader::FromRawData(shader::geometry_frag, shader::geometry_frag_len),
      "base_frag");

  u_transOffset_ = GL.GetUniformLocation(program(), "u_transOffset");
  u_texture_ = GL.GetUniformLocation(program(), "u_texture");
  u_textureEmptyFlag_ = GL.GetUniformLocation(program(), "u_textureEmptyFlag");
}

void GeometryShader::SetTransOffset(const base::Vec2& offset) {
  GL.Uniform2f(u_transOffset_, offset.x, offset.y);
}

void GeometryShader::SetTexture(GLID<Texture> tex) {
  GLES2ShaderBase::SetTexture(u_texture_, tex.gl, 1);
}

void GeometryShader::SetTextureEmptyFlag(float flag) {
  GL.Uniform1f(u_textureEmptyFlag_, flag);
}

SpineShader::SpineShader() {
  GLES2ShaderBase::Setup(
      shader::FromRawData(shader::spine_vert, shader::spine_vert_len),
      "spine_vert",
      shader::FromRawData(shader::spine_frag, shader::spine_frag_len),
      "spine_frag");

  u_transOffset_ = GL.GetUniformLocation(program(), "u_transOffset");
  u_texture_ = GL.GetUniformLocation(program(), "u_texture");
}

void SpineShader::SetTransOffset(const base::Vec2& offset) {
  GL.Uniform2f(u_transOffset_, offset.x, offset.y);
}

void SpineShader::SetTexture(GLID<Texture> tex) {
  GLES2ShaderBase::SetTexture(u_texture_, tex.gl, 1);
}

YUVShader::YUVShader() {
  GLES2ShaderBase::Setup(
      shader::FromRawData(shader::base_vert, shader::base_vert_len),
      "base_vert", shader::FromRawData(shader::yuv_frag, shader::yuv_frag_len),
      "yuv_frag");

  u_transOffset_ = GL.GetUniformLocation(program(), "u_transOffset");
  u_texSize_ = GL.GetUniformLocation(program(), "u_texSize");
  u_textureY_ = GL.GetUniformLocation(program(), "u_textureY");
  u_textureU_ = GL.GetUniformLocation(program(), "u_textureU");
  u_textureV_ = GL.GetUniformLocation(program(), "u_textureV");
}

void YUVShader::SetTextureSize(const base::Vec2& tex_size) {
  GL.Uniform2f(u_texSize_, 1.f / tex_size.x, 1.f / tex_size.y);
}

void YUVShader::SetTransOffset(const base::Vec2& offset) {
  GL.Uniform2f(u_transOffset_, offset.x, offset.y);
}

void YUVShader::SetTextureY(GLID<Texture> tex) {
  GLES2ShaderBase::SetTexture(u_textureY_, tex.gl, 1);
}

void YUVShader::SetTextureU(GLID<Texture> tex) {
  GLES2ShaderBase::SetTexture(u_textureU_, tex.gl, 2);
}

void YUVShader::SetTextureV(GLID<Texture> tex) {
  GLES2ShaderBase::SetTexture(u_textureV_, tex.gl, 3);
}

}  // namespace renderer
