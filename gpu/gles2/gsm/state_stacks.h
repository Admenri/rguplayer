// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_GLES2_GSM_STATE_STACKS_H_
#define GPU_GLES2_GSM_STATE_STACKS_H_

#include <stack>

#include "base/math/math.h"
#include "gpu/gles2/context/gles_context.h"

namespace gpu {

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

  void Set(const GLType& value) {
    if (current_ == value) return;

    Init(value);
  }

  void Push(const GLType& value) {
    stack_.push(current_);
    Set(value);
  }

  void Pop() {
    Set(stack_.top());
    stack_.pop();
  }

  void Refresh() { OnSetState(current_); }

 protected:
  virtual void OnSetState(const GLType& value) = 0;

 private:
  GLType current_;
  std::stack<GLType> stack_;
};

class GLViewport : public StateStack<base::Rect> {
 public:
  void OnSetState(const base::Rect& value);
};

class GLProgram : public StateStack<GLuint> {
 public:
  void OnSetState(const GLuint& value);
};

class GLScissorTest : public StateStack<bool> {
 public:
  void OnSetState(const bool& value);
};

class GLScissorRegion : public StateStack<base::Rect> {
 public:
  void OnSetState(const base::Rect& value);
};

class GLBlend : public StateStack<bool> {
 public:
  void OnSetState(const bool& value);
};

enum class GLBlendType {
  KeepDestAlpha = -1,

  Normal = 0,
  Addition = 1,
  Substraction = 2
};

class GLBlendFunc : public StateStack<GLBlendType> {
 public:
  void OnSetState(const GLBlendType& value);
};

class GLClearColor : public StateStack<base::Vec4> {
 public:
  void OnSetState(const base::Vec4& value);
};

}  // namespace gpu

#endif  // GPU_GLES2_GSM_STATE_STACKS_H_
