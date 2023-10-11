// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_GLES2_GSM_GLES_GSM_H_
#define GPU_GLES2_GSM_GLES_GSM_H_

#include "gpu/gles2/context/gles_context.h"
#include "gpu/gles2/gsm/state_stacks.h"

namespace gpu {

class GlobalStateManager;

extern GlobalStateManager GSM;

class GlobalStateManager final {
 public:
  GlobalStateManager() = default;

  GlobalStateManager(const GlobalStateManager&) = delete;
  GlobalStateManager& operator=(const GlobalStateManager&) = delete;

  void InitStates();

  GLViewport viewport;
  GLProgram program;
  GLScissorTest scissor;
  GLScissorRegion scissor_rect;
  GLBlend blend;
  GLBlendFunc blend_func;
};

}  // namespace gpu

#endif  // GPU_GLES2_GSM_GLES_GSM_H_
