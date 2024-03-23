// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/geometry.h"

namespace content {

Geometry::Geometry(scoped_refptr<Graphics> screen,
                   scoped_refptr<Viewport> viewport)
    : GraphicElement(screen),
      Disposable(screen),
      ViewportChild(screen, viewport) {
  triangle_count_ = 64;
  triangle_vertices_.resize(triangle_count_ * 3);
  buffer_need_update_ = true;
}

Geometry::~Geometry() {
  Dispose();
}

void Geometry::SetBitmap(scoped_refptr<Bitmap> bitmap) {
  if (bitmap_ == bitmap)
    return;
  bitmap_ = bitmap;
}

void Geometry::Resize(size_t count) {
  CheckIsDisposed();
  triangle_vertices_.resize(count * 3);
  triangle_count_ = count;
  buffer_need_update_ = true;
}

size_t Geometry::GetCapacity() {
  CheckIsDisposed();
  return triangle_count_;
}

void Geometry::SetPosition(size_t index, const base::Vec4 position) {
  CheckIsDisposed();

  SetPositionInternal(index, position);
}

void Geometry::SetTexcoord(size_t index, const base::Vec2 texcoord) {
  CheckIsDisposed();

  SetTexcoordInternal(index, texcoord);
}

void Geometry::SetColor(size_t index, const base::Vec4 color) {
  CheckIsDisposed();

  SetColorInternal(index, color);
}

void Geometry::SetShader(scoped_refptr<Shader> shader) {
  CheckIsDisposed();

  if (shader_program_ == shader)
    return;
  shader_program_ = shader;
}

void Geometry::OnObjectDisposed() {
  RemoveFromList();

  triangle_vertices_.clear();
  triangle_count_ = 0;

  renderer::VertexArray<renderer::GeometryVertex>::Uninit(vao_);
  renderer::VertexBuffer::Del(vao_.vbo);
}

void Geometry::InitDrawableData() {
  vao_.vbo = renderer::VertexBuffer::Gen();
  vao_.ibo = renderer::GLID<renderer::IndexBuffer>(0);
  renderer::VertexArray<renderer::GeometryVertex>::Init(vao_);
}

void Geometry::BeforeComposite() {
  if (buffer_need_update_) {
    buffer_need_update_ = false;

    renderer::VertexBuffer::Bind(vao_.vbo);
    size_t buffer_size = triangle_count_ * sizeof(renderer::GeometryVertex) * 3;
    if (buffer_size > vbo_size_) {
      renderer::VertexBuffer::BufferData(buffer_size, triangle_vertices_.data(),
                                         GL_DYNAMIC_DRAW);
      vbo_size_ = buffer_size;
    } else {
      // As subdata upload
      renderer::VertexBuffer::BufferSubData(0, buffer_size,
                                            triangle_vertices_.data());
    }
    renderer::VertexBuffer::Unbind();
  }
}

void Geometry::Composite() {
  const bool texture_valid = bitmap_ && !bitmap_->IsDisposed();

  if (shader_program_ && !shader_program_->IsDisposed()) {
    shader_program_->BindShader();
    shader_program_->SetInternalUniform();

    base::Vec2 trans_offset = parent_rect().GetRealOffset();
    GLint offset_location =
        shader_program_->GetUniformLocation("u_transOffset");
    GLint flag_location =
        shader_program_->GetUniformLocation("u_textureEmptyFlag");
    renderer::GL.Uniform2f(offset_location, trans_offset.x, trans_offset.y);
    renderer::GL.Uniform1f(flag_location, texture_valid ? 0.0f : 1.0f);

    if (texture_valid) {
      GLint texture_location = shader_program_->GetUniformLocation("u_texture");
      GLint texture_size_location =
          shader_program_->GetUniformLocation("u_texSize");

      renderer::GLES2ShaderBase::SetTexture(texture_location,
                                            bitmap_->GetTexture().tex.gl, 1);
      renderer::GL.Uniform2f(texture_size_location, bitmap_->GetWidth(),
                             bitmap_->GetHeight());
    }
  } else {
    auto& shader = renderer::GSM.shaders()->geometry;
    shader.Bind();
    shader.SetProjectionMatrix(renderer::GSM.states.viewport.Current().Size());
    shader.SetTransOffset(parent_rect().GetRealOffset());
    shader.SetTextureEmptyFlag(texture_valid ? 0.0f : 1.0f);
    if (texture_valid) {
      shader.SetTexture(bitmap_->GetTexture().tex);
      shader.SetTextureSize(bitmap_->GetSize());
    }
  }

  renderer::GSM.states.blend.Push(true);
  renderer::GSM.states.blend_func.PushOnly();

  if (shader_program_ && !shader_program_->IsDisposed())
    shader_program_->SetBlendFunc();
  else
    renderer::GSM.states.blend_func.Set(blend_mode_);

  renderer::VertexArray<renderer::GeometryVertex>::Bind(vao_);
  renderer::GL.DrawArrays(GL_TRIANGLES, 0,
                          (GLsizei)(triangle_vertices_.size()));
  renderer::VertexArray<renderer::GeometryVertex>::Unbind();

  renderer::GSM.states.blend_func.Pop();
  renderer::GSM.states.blend.Pop();
}

void Geometry::SetPositionInternal(size_t index, const base::Vec4 position) {
  size_t size = triangle_vertices_.size();
  index = std::clamp<size_t>(index, 0, size - 1);

  renderer::GeometryVertex* vert = triangle_vertices_.data();
  vert[index].position = position;

  buffer_need_update_ = true;
}

void Geometry::SetTexcoordInternal(size_t index, const base::Vec2 texcoord) {
  size_t size = triangle_vertices_.size();
  index = std::clamp<size_t>(index, 0, size - 1);

  renderer::GeometryVertex* vert = triangle_vertices_.data();
  vert[index].texCoord = texcoord;

  buffer_need_update_ = true;
}

void Geometry::SetColorInternal(size_t index, const base::Vec4 color) {
  size_t size = triangle_vertices_.size();
  index = std::clamp<size_t>(index, 0, size - 1);

  renderer::GeometryVertex* vert = triangle_vertices_.data();
  vert[index].color = color;

  buffer_need_update_ = true;
}

}  // namespace content
