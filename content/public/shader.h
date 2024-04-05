// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_SHADER_H_
#define CONTENT_PUBLIC_SHADER_H_

#include "base/memory/ref_counted.h"
#include "content/public/disposable.h"
#include "content/public/graphics.h"
#include "renderer/context/gles2_context.h"
#include "renderer/thread/thread_manager.h"

#include <unordered_map>

namespace content {

class Shader : public base::RefCounted<Shader>,
               public GraphicElement,
               public Disposable {
 public:
  Shader(scoped_refptr<Graphics> screen);
  ~Shader();

  Shader(const Shader&) = delete;
  Shader& operator=(const Shader&) = delete;

  void Compile(const std::string& vertex_shader,
               const std::string& fragment_shader);
  void Reset();
  void SetBlend(GLenum mode,
                GLenum srcRGB,
                GLenum dstRGB,
                GLenum srcAlpha,
                GLenum dstAlpha);

  void SetParam(const std::string& uniform,
                const std::vector<float>& params,
                int count);
  void SetParam(const std::string& uniform,
                const std::vector<int>& params,
                int count);
  void SetParam(const std::string& uniform,
                const std::vector<float>& matrix,
                int count,
                bool transpose);
  void SetParam(const std::string& uniform,
                scoped_refptr<Bitmap> texture,
                int index);

 private:
  friend class Geometry;
  friend class Viewport;
  friend class Graphics;

  void OnObjectDisposed() override;
  std::string_view DisposedObjectName() const override { return "Shader"; }

  void CompileInternal(const std::string& vertex_shader,
                       const std::string& fragment_shader);
  void ResetInternal();
  void LinkProgramInternal();
  void SetParam1Internal(const std::string& uniform,
                         const std::vector<float>& params,
                         int count);
  void SetParam2Internal(const std::string& uniform,
                         const std::vector<int>& params,
                         int count);
  void SetParam3Internal(const std::string& uniform,
                         const std::vector<float>& matrix,
                         int count,
                         bool transpose);
  void SetParam4Internal(const std::string& uniform,
                         scoped_refptr<Bitmap> texture,
                         int index);

  // Used in shader drawable
  void BindShader();
  void SetInternalUniform();
  void SetBlendFunc();
  GLint GetUniformLocation(const std::string& name);

  struct TextureUnit {
    scoped_refptr<Bitmap> texture = nullptr;
    GLint location = -1;
  };

  std::unordered_map<int, TextureUnit> bind_textures_;
  GLuint vertex_shader_, frag_shader_, program_;
  GLenum equal_mode_, srcRGB_, dstRGB_, srcAlpha_, dstAlpha_;
  std::unordered_map<std::string, GLint> location_cache_;
};

}  // namespace content

#endif  //! CONTENT_PUBLIC_SHADER_H_
