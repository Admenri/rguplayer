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
    GrayShader gray;
    FlatShader flat;
    AlphaTransShader alpha_trans;
    VagueTransShader vague_shader;
    FlashTileShader flash_tile;
    Tilemap2Shader tilemap2;
  };

  GlobalStateManager() = default;

  GlobalStateManager(const GlobalStateManager&) = delete;
  GlobalStateManager& operator=(const GlobalStateManager&) = delete;

  void InitStates();
  void QuitStates();

  void EnsureGenericTex(int width, int height);

  struct {
    GLViewport viewport;
    GLProgram program;
    GLScissorTest scissor;
    GLScissorRegion scissor_rect;
    GLBlend blend;
    GLBlendFunc blend_func;
    GLClearColor clear_color;
  } states;

  std::unique_ptr<GLShaderWare> shaders;

  TextureFrameBuffer common_tfb;
  std::unique_ptr<QuadDrawable> common_quad;

  std::unique_ptr<QuadIndexBuffer> quad_ibo;
};

}  // namespace renderer

#endif  // !RENDERER_THREAD_THREAD_MANAGER_H_
