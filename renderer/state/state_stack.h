// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RENDERER_STATE_STATE_STACK_H_
#define RENDERER_STATE_STATE_STACK_H_

#include <stack>

#include "base/math/math.h"
#include "base/memory/ref_counted.h"
#include "gpu/gl_forward.h"
#include "renderer/renderer_utility.h"

namespace renderer {

template <typename T>
class GLState {
 public:
  GLState(scoped_refptr<gpu::GLES2CommandContext> gl_context)
      : context_(gl_context), current_(T()) {}
  virtual ~GLState() {}

  GLState(const GLState&) = delete;
  GLState& operator=(const GLState&) = delete;

  void Set(T value) {
    current_ = value;
    OnApplyProperty(current_);
  }

  void Push(const T& value) {
    stack_.push(value);
    Set(value);
  }

  void Pop() {
    if (stack_.empty()) return;
    stack_.pop();
    Set(stack_.top());
  }

  T Current() { return current_; }

 protected:
  virtual void OnApplyProperty(const T& value) = 0;

  scoped_refptr<gpu::GLES2CommandContext> GetContext() { return context_; }

 private:
  scoped_refptr<gpu::GLES2CommandContext> context_;

  std::stack<T> stack_;
  T current_;
};

class GLViewport : public GLState<base::Rect> {
 public:
  GLViewport(scoped_refptr<gpu::GLES2CommandContext> gl_context)
      : GLState(gl_context) {}

 protected:
  void OnApplyProperty(const base::Rect& value) override;
};

class GLScissorRegion : public GLState<base::Rect> {
 public:
  GLScissorRegion(scoped_refptr<gpu::GLES2CommandContext> gl_context)
      : GLState(gl_context) {}

 protected:
  void OnApplyProperty(const base::Rect& value) override;
};

class GLScissorTest : public GLState<bool> {
 public:
  GLScissorTest(scoped_refptr<gpu::GLES2CommandContext> gl_context)
      : GLState(gl_context) {}

 protected:
  void OnApplyProperty(const bool& value) override;
};

class GLBlend : public GLState<bool> {
 public:
  GLBlend(scoped_refptr<gpu::GLES2CommandContext> gl_context)
      : GLState(gl_context) {}

 protected:
  void OnApplyProperty(const bool& value) override;
};

class GLBlendMode : public GLState<BlendMode> {
 public:
  GLBlendMode(scoped_refptr<gpu::GLES2CommandContext> gl_context)
      : GLState(gl_context) {}

 protected:
  void OnApplyProperty(const BlendMode& value) override;
};

}  // namespace renderer

#endif  // RENDERER_STATE_STATE_STACK_H_