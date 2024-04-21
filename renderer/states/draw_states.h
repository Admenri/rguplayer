// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef RENDERER_STATES_DRAW_STATES_H_
#define RENDERER_STATES_DRAW_STATES_H_

#include "base/math/math.h"
#include "renderer/meta/gles2meta.h"

#include "SDL_opengl.h"

#include <stack>

namespace renderer {

template <typename GLType>
class StateStack {
 public:
  StateStack() = default;

  StateStack(const StateStack&) = delete;
  StateStack& operator=(const StateStack&) = delete;

  GLType Current() { return current_; }

  void Init(const GLType& value) {
    current_ = value;
    OnSetState(value);
  }

  inline void Set(const GLType& value) {
    if (current_ == value)
      return;

    Init(value);
  }

  inline void Push(const GLType& value) {
    stack_.push(current_);
    Set(value);
  }

  inline void PushOnly() { stack_.push(current_); }

  inline void Pop() {
    Set(stack_.top());
    stack_.pop();
  }

  inline void Refresh() { OnSetState(current_); }

 protected:
  virtual void OnSetState(const GLType& value) = 0;

 private:
  GLType current_;
  std::stack<GLType> stack_;
};

class GLViewport : public StateStack<base::Rect> {
 public:
  void OnSetState(const base::Rect& value) override;
};

class GLProgram : public StateStack<GLuint> {
 public:
  void OnSetState(const GLuint& value) override;
};

class GLScissorTest : public StateStack<bool> {
 public:
  void OnSetState(const bool& value) override;
};

class GLScissorRegion : public StateStack<base::Rect> {
 public:
  void OnSetState(const base::Rect& value) override;

  void SetIntersect(const base::Rect& value);
};

class GLBlend : public StateStack<bool> {
 public:
  void OnSetState(const bool& value) override;
};

enum class GLBlendType {
  KeepDestAlpha = -1,

  Normal = 0,
  Addition = 1,
  Substraction = 2
};

class GLBlendFunc : public StateStack<GLBlendType> {
 public:
  void OnSetState(const GLBlendType& value) override;
};

class GLClearColor : public StateStack<base::Vec4> {
 public:
  void OnSetState(const base::Vec4& value) override;
};

}  // namespace renderer

#endif  // !RENDERER_STATES_DRAW_STATES_H_
