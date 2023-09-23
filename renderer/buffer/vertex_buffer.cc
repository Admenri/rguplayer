// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "renderer/buffer/vertex_buffer.h"

namespace renderer {

#define VertexAttrDefine(t)                                               \
  const GLVertexAttribute* VertexAttributeTraits<CommonVertex>::attr = t; \
  size_t VertexAttributeTraits<CommonVertex>::attr_size =                 \
      sizeof(t) / sizeof(t[0]);

const GLVertexAttribute CommonVertexAttr[] = {
    {
        gpu::ShaderLocation::Position,
        2,
        GL_FLOAT,
        reinterpret_cast<const GLvoid*>(offsetof(CommonVertex, position)),
    },
    {
        gpu::ShaderLocation::TexCoord,
        2,
        GL_FLOAT,
        reinterpret_cast<const GLvoid*>(offsetof(CommonVertex, texcoord)),
    },
    {
        gpu::ShaderLocation::Color,
        4,
        GL_FLOAT,
        reinterpret_cast<const GLvoid*>(offsetof(CommonVertex, color)),
    },
};
VertexAttrDefine(CommonVertexAttr);

}  // namespace renderer
