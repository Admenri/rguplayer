// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "renderer/vertex/vertex_set.h"

namespace renderer {

static const VertexItemAttribute CommonVertexInfo[] = {
    {GLES2Shader::AttribLocation::Position, 4, GL_FLOAT,
     (const void*)offsetof(CommonVertex, position)},
    {GLES2Shader::AttribLocation::TexCoord, 2, GL_FLOAT,
     (const void*)offsetof(CommonVertex, texCoord)},
    {GLES2Shader::AttribLocation::Color, 4, GL_FLOAT,
     (const void*)offsetof(CommonVertex, color)}};

template <>
const VertexItemAttribute* VertexInfo<CommonVertex>::attrs = CommonVertexInfo;
template <>
const int VertexInfo<CommonVertex>::attr_size = sizeof(CommonVertexInfo) /
                                                sizeof(CommonVertexInfo[0]);

}  // namespace renderer
