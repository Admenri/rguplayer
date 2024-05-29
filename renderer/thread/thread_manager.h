// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef RENDERER_THREAD_THREAD_MANAGER_H_
#define RENDERER_THREAD_THREAD_MANAGER_H_

#include "renderer/shader/gles2_shaders.h"
#include "renderer/states/draw_states.h"

namespace renderer {

class QuadDrawable;
class GlobalStateManager;
class QuadIndexBuffer;

thread_local extern GlobalStateManager GSM;

class GlobalStateManager final {
 public:
  struct GLShaderWare {
    BaseShader base;
    SpriteShader sprite;
    TexBltShader texblt;
    ColorShader color;
    PlaneShader plane;
    BaseAlphaShader base_alpha;
    ViewportShader viewport;
    FlatShader flat;
    AlphaTransShader alpha_trans;
    VagueTransShader vague_shader;
    FlashTileShader flash_tile;
    Tilemap2Shader tilemap2;
    HueShader hue;
    TilemapShader tilemap;
    BaseSpriteShader basesprite;
    AlphaSpriteShader alphasprite;
    GeometryShader geometry;
    SpineShader spine;
    YUVShader yuv;
  };

  GlobalStateManager() = default;

  GlobalStateManager(const GlobalStateManager&) = delete;
  GlobalStateManager& operator=(const GlobalStateManager&) = delete;

  void InitStates();
  void QuitStates();

  TextureFrameBuffer& EnsureCommonTFB(int width, int height);
  GLID<Texture>& EnsureGenericTex(int width, int height, base::Vec2i& out_size);

  struct {
    GLViewport viewport;
    GLProgram program;
    GLScissorTest scissor;
    GLScissorRegion scissor_rect;
    GLBlend blend;
    GLBlendFunc blend_func;
    GLClearColor clear_color;
    GLVertexAttrib vertex_attrib;
  } states;

  GLShaderWare* shaders() const { return shaders_.get(); }
  QuadDrawable* common_quad() const { return common_quad_.get(); }
  QuadIndexBuffer* quad_ibo() const { return quad_ibo_.get(); }
  int& max_texture_size() { return max_texture_size_; }
  bool& enable_es_shaders() { return enable_es_shaders_; }

 private:
  std::unique_ptr<GLShaderWare> shaders_;

  TextureFrameBuffer common_tfb_;
  std::unique_ptr<QuadDrawable> common_quad_;
  std::unique_ptr<QuadIndexBuffer> quad_ibo_;

  GLID<Texture> generic_tex_;
  base::Vec2i generic_tex_size_;

  int max_texture_size_;
  bool enable_es_shaders_;
};

}  // namespace renderer

#endif  // !RENDERER_THREAD_THREAD_MANAGER_H_
