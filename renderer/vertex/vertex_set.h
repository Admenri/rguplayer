// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef RENDERER_VERTEX_VERTEX_SET_H_
#define RENDERER_VERTEX_VERTEX_SET_H_

#include <stdint.h>

#include "base/math/math.h"
#include "renderer/shader/gles2_shaders.h"

namespace renderer {

struct CommonVertex {
  base::Vec2 position;
  base::Vec2 texCoord;

  /* Default color for alpha composite */
  base::Vec4 color{0, 0, 0, 1.0f};
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

template <typename VertexType>
struct VertexArray {
  using Type = VertexType;

  // ARB_vertex_arrays
  GLID<VertexArray> id;

  GLID<VertexBuffer> vbo;
  GLID<IndexBuffer> ibo;

  inline static void SetAttrib(VertexArray& vao) {
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
    if (GL.GenVertexArraysOES) {
      GL.GenVertexArraysOES(1, &vao.id.gl);
      GL.BindVertexArrayOES(vao.id.gl);

      SetAttrib(vao);

      GL.BindVertexArrayOES(0);
    }
  }

  inline static void Uninit(VertexArray& vao) {
    if (GL.GenVertexArraysOES) {
      GL.DeleteVertexArraysOES(1, &vao.id.gl);
    }
  }

  inline static void Bind(VertexArray& vao) {
    if (GL.GenVertexArraysOES) {
      GL.BindVertexArrayOES(vao.id.gl);
    } else {
      SetAttrib(vao);
    }
  }

  inline static void Unbind() {
    if (GL.GenVertexArraysOES) {
      GL.BindVertexArrayOES(0);
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
