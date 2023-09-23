#ifndef RENDERER_OGL_GL_STATESTACK_H_
#define RENDERER_OGL_GL_STATESTACK_H_

#include <memory>
#include <stack>

#include "base/math/math.h"
#include "gpu/gles2/gles_context.h"
#include "renderer/renderer_utility.h"

namespace renderer {

template <typename T>
class GLState {
 public:
  GLState(std::shared_ptr<gpu::GLES2CommandContext> gl_context)
      : context_(gl_context) {}
  virtual ~GLState() {}

  GLState(const GLState&) = delete;
  GLState& operator=(const GLState&) = delete;

  void Set(const T& value) {
    current_ = value;
    OnApplyProperty(value);
  }

  void Push(const T& value) {
    stack_.push(value);
    ApplyTop();
  }

  void Pop() {
    stack_.pop();
    ApplyTop();
  }

  T& Current() { return current_; }

 protected:
  virtual void OnApplyProperty(const T& value) = 0;

  std::shared_ptr<gpu::GLES2CommandContext> GetContext() { return context_; }

 private:
  void ApplyTop();

  std::shared_ptr<gpu::GLES2CommandContext> context_;

  T current_;
  std::stack<T> stack_;
};

class GLViewport : public GLState<base::Rect> {
 public:
  GLViewport(std::shared_ptr<gpu::GLES2CommandContext> gl_context)
      : GLState(gl_context) {}

 protected:
  void OnApplyProperty(const base::Rect& value) override;
};

class GLScissorRegion : public GLState<base::Rect> {
 public:
  GLScissorRegion(std::shared_ptr<gpu::GLES2CommandContext> gl_context)
      : GLState(gl_context) {}

 protected:
  void OnApplyProperty(const base::Rect& value) override;
};

class GLScissorTest : public GLState<bool> {
 public:
  GLScissorTest(std::shared_ptr<gpu::GLES2CommandContext> gl_context)
      : GLState(gl_context) {}

 protected:
  void OnApplyProperty(const bool& value) override;
};

class GLBlendMode : public GLState<BlendMode> {
 public:
  GLBlendMode(std::shared_ptr<gpu::GLES2CommandContext> gl_context)
      : GLState(gl_context) {}

 protected:
  void OnApplyProperty(const BlendMode& value) override;
};

class GLProgram : public GLState<GLuint> {
 public:
  GLProgram(std::shared_ptr<gpu::GLES2CommandContext> gl_context)
      : GLState(gl_context) {}

 protected:
  void OnApplyProperty(const GLuint& value) override;
};

}  // namespace renderer

#endif  // GL_STATESTACK_H_