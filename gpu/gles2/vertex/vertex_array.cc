// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/gles2/vertex/vertex_array.h"

namespace gpu {

static const VertexItemAttribute CommonVertexInfo[] = {
    {ShaderAttribLocation::Position, 4, GL_FLOAT,
     (const void *)offsetof(CommonVertex, position)},
    {ShaderAttribLocation::TexCoord, 2, GL_FLOAT,
     (const void *)offsetof(CommonVertex, texCoord)},
    {ShaderAttribLocation::Color, 4, GL_FLOAT,
     (const void *)offsetof(CommonVertex, color)}};

template <>
const VertexItemAttribute *VertexInfo<CommonVertex>::attrs = CommonVertexInfo;
template <>
const int VertexInfo<CommonVertex>::attr_size = sizeof(CommonVertexInfo) /
                                                sizeof(CommonVertexInfo[0]);

}  // namespace gpu
