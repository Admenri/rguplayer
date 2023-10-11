// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/gles2/gsm/state_stacks.h"

namespace gpu {

void GLViewport::OnSetState(const base::Rect& value) {
  GL.Viewport(value.x, value.y, value.width, value.height);
}

void GLProgram::OnSetState(const GLuint& value) { GL.UseProgram(value); }

void GLScissorTest::OnSetState(const bool& value) {
  value ? GL.Enable(GL_SCISSOR_TEST) : GL.Disable(GL_SCISSOR_TEST);
}

void GLScissorRegion::OnSetState(const base::Rect& value) {
  GL.Scissor(value.x, value.y, value.width, value.height);
}

void GLBlend::OnSetState(const bool& value) {
  value ? GL.Enable(GL_BLEND) : GL.Disable(GL_BLEND);
}

void GLBlendFunc::OnSetState(const GLBlendType& value) {
  switch (value) {
    case GLBlendType::KeepDestAlpha:
      GL.BlendEquation(GL_FUNC_ADD);
      GL.BlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO,
                           GL_ONE);
      break;

    case GLBlendType::Normal:
      GL.BlendEquation(GL_FUNC_ADD);
      GL.BlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,
                           GL_ONE_MINUS_SRC_ALPHA);
      break;

    case GLBlendType::Addition:
      GL.BlendEquation(GL_FUNC_ADD);
      GL.BlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE);
      break;

    case GLBlendType::Substraction:
      GL.BlendEquation(GL_FUNC_REVERSE_SUBTRACT);
      GL.BlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_ONE);
      break;
  }
}

}  // namespace gpu
