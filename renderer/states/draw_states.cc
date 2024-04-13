// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "renderer/states/draw_states.h"

#include "renderer/context/gles2_context.h"

#include "SDL_rect.h"

namespace renderer {

void GLViewport::OnSetState(const base::Rect& value) {
  GL.Viewport(value.x, value.y, value.width, value.height);
}

void GLProgram::OnSetState(const GLuint& value) {
  GL.UseProgram(value);
}

void GLScissorTest::OnSetState(const bool& value) {
  value ? GL.Enable(GL_SCISSOR_TEST) : GL.Disable(GL_SCISSOR_TEST);
}

void GLScissorRegion::OnSetState(const base::Rect& value) {
  GL.Scissor(value.x, value.y, value.width, value.height);
}

void GLScissorRegion::SetIntersect(const base::Rect& value) {
  const base::Rect& current = Current();

  SDL_Rect r1 = {current.x, current.y, current.width, current.height};
  SDL_Rect r2 = {value.x, value.y, value.width, value.height};

  SDL_Rect result;
  if (!SDL_GetRectIntersection(&r1, &r2, &result))
    result.w = result.h = 0;

  Set(base::Rect(result.x, result.y, result.w, result.h));
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

void GLClearColor::OnSetState(const base::Vec4& value) {
  GL.ClearColor(value.x, value.y, value.z, value.w);
}

void GLVertexAttrib::OnSetState(const GLID<VertexAttrib>& value) {
  if (GL.BindVertexArray)
    GL.BindVertexArray(value.gl);
}

}  // namespace renderer
