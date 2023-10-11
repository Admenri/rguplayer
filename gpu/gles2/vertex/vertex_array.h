// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_GLES2_VERTEX_VERTEX_ARRAY_H_
#define GPU_GLES2_VERTEX_VERTEX_ARRAY_H_

#include <stdint.h>

#include "base/math/math.h"
#include "gpu/gles2/context/gles_context.h"
#include "gpu/gles2/meta/gles_meta.h"
#include "gpu/gles2/shader/shader_manager.h"

namespace gpu {

struct CommonVertex {
  base::Vec2 position;
  base::Vec2 texCoord;
  base::Vec4 color;
};

struct VertexItemAttribute {
  ShaderAttribLocation index;
  int32_t size;
  GLenum type;
  const GLvoid* offset;
};

template <typename ItemAttr>
struct VertexInfo {
  static const VertexItemAttribute* attrs;
  static const int attr_size;
};

template <typename VertexType>
struct VertexArray {
  using Type = VertexType;

  // ARB_vertex_arrays
  GLID<VertexArray> id;

  static void Init(VertexArray& vao);

  inline static void Uninit(VertexArray& vao) {
    GL.DeleteVertexArrays(1, &vao.id);
  }

  inline static void Bind(VertexArray& vao) { GL.BindVertexArray(vao.id.gl); }
  inline static void Unbind(VertexArray& vao) { GL.BindVertexArray(0); }
};

template <typename VertexType>
inline void VertexArray<VertexType>::Init(VertexArray& vao) {
  GL.GenVertexArrays(1, &vao.id.gl);
  GL.BindVertexArray(vao.id.gl);

  for (size_t i = 0; i < VertexInfo<Type>::attr_size; i++) {
    GL.EnableVertexAttribArray(VertexInfo<Type>::attrs[i].index);
    GL.VertexAttribPointer(VertexInfo<Type>::attrs[i].index,
                           VertexInfo<Type>::attrs[i].size,
                           VertexInfo<Type>::attrs[i].type, GL_FALSE,
                           sizeof(Type), VertexInfo<Type>::attrs[i].offset);
  }
}

}  // namespace gpu

#endif  // GPU_GLES2_VERTEX_VERTEX_ARRAY_H_