// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef RENDERER_VERTEX_VERTEX_SET_H_
#define RENDERER_VERTEX_VERTEX_SET_H_

#include <stdint.h>

#include "base/math/math.h"
#include "renderer/shader/gles2_shaders.h"
#include "renderer/thread/thread_manager.h"

namespace renderer {

struct CommonVertex {
  base::Vec2 position;
  base::Vec2 texCoord;

  /* Default color for alpha composite */
  base::Vec4 color{0, 0, 0, 1.0f};
};

struct GeometryVertex {
  base::Vec4 position;
  base::Vec2 texCoord;
  base::Vec4 color;
};

struct VertexItemAttribute {
  GLES2Shader::AttribLocation index;
  int32_t size;
  GLenum type;
  const GLvoid* offset;
};

template <typename ItemAttr>
struct VertexInfo {
  static const VertexItemAttribute* attrs;
  static const int attr_size;
};

extern const VertexItemAttribute CommonVertexInfo[];
extern const int CommonVertexInfoSize;

extern const VertexItemAttribute GeometryVertexInfo[];
extern const int GeometryVertexInfoSize;

template <>
const VertexItemAttribute* VertexInfo<CommonVertex>::attrs = CommonVertexInfo;
template <>
const int VertexInfo<CommonVertex>::attr_size = CommonVertexInfoSize;

template <>
const VertexItemAttribute* VertexInfo<GeometryVertex>::attrs =
    GeometryVertexInfo;
template <>
const int VertexInfo<GeometryVertex>::attr_size = GeometryVertexInfoSize;

template <typename VertexType>
struct VertexArray {
  using Type = VertexType;

  // ARB_vertex_arrays
  GLID<VertexAttrib> id;

  GLID<VertexBuffer> vbo;
  GLID<IndexBuffer> ibo;

  inline static void SetAttrib(const VertexArray& vao) {
    VertexBuffer::Bind(vao.vbo);
    IndexBuffer::Bind(vao.ibo);

    for (size_t i = 0; i < VertexInfo<Type>::attr_size; i++) {
      GL.EnableVertexAttribArray(VertexInfo<Type>::attrs[i].index);
      GL.VertexAttribPointer(VertexInfo<Type>::attrs[i].index,
                             VertexInfo<Type>::attrs[i].size,
                             VertexInfo<Type>::attrs[i].type, GL_FALSE,
                             sizeof(Type), VertexInfo<Type>::attrs[i].offset);
    }
  }

  inline static void Init(VertexArray& vao) {
    if (GL.GenVertexArrays) {
      GL.GenVertexArrays(1, &vao.id.gl);
      GL.BindVertexArray(vao.id.gl);
      SetAttrib(vao);
      GL.BindVertexArray(0);
    }
  }

  inline static void Uninit(const VertexArray& vao) {
    if (GL.GenVertexArrays) {
      GL.DeleteVertexArrays(1, &vao.id.gl);
    }
  }

  inline static void Bind(const VertexArray& vao) {
    if (GL.GenVertexArrays) {
      GL.BindVertexArray(vao.id.gl);
    } else {
      SetAttrib(vao);
    }
  }

  inline static void Unbind() {
    if (GL.GenVertexArrays) {
      GL.BindVertexArray(0);
    } else {
      for (size_t i = 0; i < VertexInfo<Type>::attr_size; i++) {
        GL.DisableVertexAttribArray(VertexInfo<Type>::attrs[i].index);
      }

      IndexBuffer::Unbind();
      VertexBuffer::Unbind();
    }
  }
};

}  // namespace renderer

#endif  // !RENDERER_VERTEX_VERTEX_SET_H_
