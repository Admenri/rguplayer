// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "renderer/state/state_stack.h"

namespace renderer {

void GLViewport::OnApplyProperty(const base::Rect& value) {
  GetContext()->glViewport(value.x, value.y, value.width, value.height);
}

void GLScissorRegion::OnApplyProperty(const base::Rect& value) {
  GetContext()->glScissor(value.x, value.y, value.width, value.height);
}

void GLScissorTest::OnApplyProperty(const bool& value) {
  value ? GetContext()->glEnable(GL_SCISSOR_TEST)
        : GetContext()->glDisable(GL_SCISSOR_TEST);
}

void GLBlendMode::OnApplyProperty(const BlendMode& value) {
  switch (value) {
    case BlendMode::Normal: {
      GetContext()->glBlendEquation(GL_FUNC_ADD);
      GetContext()->glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
                                        GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
      break;
    }
    case BlendMode::Addition: {
      GetContext()->glBlendEquation(GL_FUNC_ADD);
      GetContext()->glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE);
      break;
    }
    case BlendMode::Substraction: {
      GetContext()->glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
      GetContext()->glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_ONE);
      break;
    }
  }
}

}  // namespace renderer
