// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RENDERER_PIPELINE_RENDER_PIPELINE_H_
#define RENDERER_PIPELINE_RENDER_PIPELINE_H_

#include "bgfx/bgfx.h"
#include "bgfx/embedded_shader.h"

#include <stdint.h>
#include <string>

namespace renderer {

class RenderShaderBase {
 public:
  virtual ~RenderShaderBase();

  RenderShaderBase(const RenderShaderBase&) = delete;
  RenderShaderBase& operator=(const RenderShaderBase&) = delete;

  bgfx::ProgramHandle GetProgram();

 protected:
  RenderShaderBase();
  void CompileProgram(const bgfx::EmbeddedShader* vert_shader,
                      const std::string& vert_name,
                      const bgfx::EmbeddedShader* frag_shader,
                      const std::string& frag_name);

 private:
  bgfx::ProgramHandle program_;
};

class BaseShader : public RenderShaderBase {
 public:
  BaseShader();
  ~BaseShader() override;

  bgfx::UniformHandle OffsetTexSize() { return u_offsetTexSize_; }
  bgfx::UniformHandle Texture() { return u_texture_; }

 private:
  bgfx::UniformHandle u_offsetTexSize_;
  bgfx::UniformHandle u_texture_;
};

class TexbltShader : public RenderShaderBase {
 public:
  TexbltShader();
  ~TexbltShader() override;

  bgfx::UniformHandle OffsetTexSize() { return u_offsetTexSize_; }
  bgfx::UniformHandle Texture() { return u_texture_; }
  bgfx::UniformHandle DstTexture() { return u_dstTexture_; }
  bgfx::UniformHandle OffsetScale() { return u_offsetScale_; }
  bgfx::UniformHandle Opacity() { return u_opacity_; }

 private:
  bgfx::UniformHandle u_offsetTexSize_;
  bgfx::UniformHandle u_texture_;
  bgfx::UniformHandle u_dstTexture_;
  bgfx::UniformHandle u_offsetScale_;
  bgfx::UniformHandle u_opacity_;
};

class BaseColorShader : public RenderShaderBase {
 public:
  BaseColorShader();
};

class HueShader : public RenderShaderBase {
 public:
  HueShader();
  ~HueShader() override;

  bgfx::UniformHandle OffsetTexSize() { return u_offsetTexSize_; }
  bgfx::UniformHandle Texture() { return u_texture_; }
  bgfx::UniformHandle HueAdjustValue() { return u_hueAdjustValue_; }

 private:
  bgfx::UniformHandle u_offsetTexSize_;
  bgfx::UniformHandle u_texture_;
  bgfx::UniformHandle u_hueAdjustValue_;
};

class ViewportShader : public RenderShaderBase {
 public:
  ViewportShader();
  ~ViewportShader() override;

  bgfx::UniformHandle OffsetTexSize() { return u_offsetTexSize_; }
  bgfx::UniformHandle Texture() { return u_texture_; }
  bgfx::UniformHandle Color() { return u_color_; }
  bgfx::UniformHandle Tone() { return u_tone_; }

 private:
  bgfx::UniformHandle u_offsetTexSize_;
  bgfx::UniformHandle u_texture_;
  bgfx::UniformHandle u_color_;
  bgfx::UniformHandle u_tone_;
};

class SpriteShader : public RenderShaderBase {
 public:
  SpriteShader();
  ~SpriteShader() override;

  bgfx::UniformHandle Transform() { return u_transformMat_; }
  bgfx::UniformHandle OffsetTexSize() { return u_offsetTexSize_; }
  bgfx::UniformHandle Texture() { return u_texture_; }
  bgfx::UniformHandle Color() { return u_color_; }
  bgfx::UniformHandle Tone() { return u_tone_; }
  bgfx::UniformHandle DrawInfo() { return u_drawInfo_; }

 private:
  bgfx::UniformHandle u_transformMat_;
  bgfx::UniformHandle u_offsetTexSize_;
  bgfx::UniformHandle u_texture_;
  bgfx::UniformHandle u_color_;
  bgfx::UniformHandle u_tone_;
  bgfx::UniformHandle u_drawInfo_;
};

class AlphaSpriteShader : public RenderShaderBase {
 public:
  AlphaSpriteShader();
  ~AlphaSpriteShader() override;

  bgfx::UniformHandle Transform() { return u_transformMat_; }
  bgfx::UniformHandle OffsetTexSize() { return u_offsetTexSize_; }
  bgfx::UniformHandle Texture() { return u_texture_; }
  bgfx::UniformHandle Opacity() { return u_opacity_; }

 private:
  bgfx::UniformHandle u_transformMat_;
  bgfx::UniformHandle u_offsetTexSize_;
  bgfx::UniformHandle u_texture_;
  bgfx::UniformHandle u_opacity_;
};

class BaseSpriteShader : public RenderShaderBase {
 public:
  BaseSpriteShader();
  ~BaseSpriteShader() override;

  bgfx::UniformHandle Transform() { return u_transformMat_; }
  bgfx::UniformHandle OffsetTexSize() { return u_offsetTexSize_; }
  bgfx::UniformHandle Texture() { return u_texture_; }

 private:
  bgfx::UniformHandle u_transformMat_;
  bgfx::UniformHandle u_offsetTexSize_;
  bgfx::UniformHandle u_texture_;
};

class PlaneShader : public RenderShaderBase {
 public:
  PlaneShader();
  ~PlaneShader() override;

  bgfx::UniformHandle OffsetTexSize() { return u_offsetTexSize_; }
  bgfx::UniformHandle Texture() { return u_texture_; }
  bgfx::UniformHandle Color() { return u_color_; }
  bgfx::UniformHandle Tone() { return u_tone_; }
  bgfx::UniformHandle Opacity() { return u_opacity_; }

 private:
  bgfx::UniformHandle u_offsetTexSize_;
  bgfx::UniformHandle u_texture_;
  bgfx::UniformHandle u_color_;
  bgfx::UniformHandle u_tone_;
  bgfx::UniformHandle u_opacity_;
};

class AlphaFlatShader : public RenderShaderBase {
 public:
  AlphaFlatShader();
  ~AlphaFlatShader() override;

  bgfx::UniformHandle Alpha() { return u_alpha_; }

 private:
  bgfx::UniformHandle u_alpha_;
};

class BaseAlphaShader : public RenderShaderBase {
 public:
  BaseAlphaShader();
  ~BaseAlphaShader() override;

  bgfx::UniformHandle OffsetTexSize() { return u_offsetTexSize_; }
  bgfx::UniformHandle Texture() { return u_texture_; }

 private:
  bgfx::UniformHandle u_offsetTexSize_;
  bgfx::UniformHandle u_texture_;
};

class TilemapShader : public RenderShaderBase {
 public:
  TilemapShader();
  ~TilemapShader() override;

  bgfx::UniformHandle OffsetTexSize() { return u_offsetTexSize_; }
  bgfx::UniformHandle Texture() { return u_texture_; }
  bgfx::UniformHandle TileSize_AnimID() { return u_tileSize_AnimateIndex_; }

 private:
  bgfx::UniformHandle u_offsetTexSize_;
  bgfx::UniformHandle u_texture_;
  bgfx::UniformHandle u_tileSize_AnimateIndex_;
};

class Tilemap2Shader : public RenderShaderBase {
 public:
  Tilemap2Shader();
  ~Tilemap2Shader() override;

  bgfx::UniformHandle OffsetTexSize() { return u_offsetTexSize_; }
  bgfx::UniformHandle Texture() { return u_texture_; }
  bgfx::UniformHandle TileSize() { return u_tileSize_; }
  bgfx::UniformHandle AnimOffset() { return u_autotileAnimationOffset_; }

 private:
  bgfx::UniformHandle u_offsetTexSize_;
  bgfx::UniformHandle u_texture_;
  bgfx::UniformHandle u_tileSize_;
  bgfx::UniformHandle u_autotileAnimationOffset_;
};

class AlphaTransShader : public RenderShaderBase {
 public:
  AlphaTransShader();
  ~AlphaTransShader() override;

  bgfx::UniformHandle OffsetTexSize() { return u_offsetTexSize_; }
  bgfx::UniformHandle FrozenTexture() { return u_frozenTexture_; }
  bgfx::UniformHandle CurrentTexture() { return u_currentTexture_; }
  bgfx::UniformHandle Progress() { return u_progress_; }

 private:
  bgfx::UniformHandle u_offsetTexSize_;
  bgfx::UniformHandle u_frozenTexture_;
  bgfx::UniformHandle u_currentTexture_;
  bgfx::UniformHandle u_progress_;
};

class VagueTransShader : public RenderShaderBase {
 public:
  VagueTransShader();
  ~VagueTransShader() override;

  bgfx::UniformHandle OffsetTexSize() { return u_offsetTexSize_; }
  bgfx::UniformHandle FrozenTexture() { return u_frozenTexture_; }
  bgfx::UniformHandle CurrentTexture() { return u_currentTexture_; }
  bgfx::UniformHandle TransTexture() { return u_transTexture_; }
  bgfx::UniformHandle ProgressVague() { return u_progressVague_; }

 private:
  bgfx::UniformHandle u_offsetTexSize_;
  bgfx::UniformHandle u_frozenTexture_;
  bgfx::UniformHandle u_currentTexture_;
  bgfx::UniformHandle u_transTexture_;
  bgfx::UniformHandle u_progressVague_;
};

}  // namespace renderer

#endif  //! RENDERER_PIPELINE_RENDER_PIPELINE_H_
