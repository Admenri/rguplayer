// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_GEOMETRY_H_
#define CONTENT_PUBLIC_GEOMETRY_H_

#include "content/public/bitmap.h"
#include "content/public/disposable.h"
#include "content/public/drawable.h"
#include "content/public/viewport.h"
#include "renderer/draw/drawable.h"

namespace content {

class Geometry : public base::RefCounted<Geometry>,
                 public GraphicElement,
                 public Disposable,
                 public ViewportChild {
 public:
  Geometry(scoped_refptr<Graphics> screen,
           scoped_refptr<Viewport> viewport = nullptr);
  ~Geometry();

  Geometry(const Geometry&) = delete;
  Geometry& operator=(const Geometry&) = delete;

  scoped_refptr<Bitmap> GetBitmap() const { return bitmap_; }
  void SetBitmap(scoped_refptr<Bitmap> bitmap);

  void Resize(size_t count);
  size_t GetCapacity();
  void SetPosition(size_t index, const base::Vec4 position);
  void SetTexcoord(size_t index, const base::Vec2 texcoord);
  void SetColor(size_t index, const base::Vec4 color);

  void SetBlendType(renderer::GLBlendType blend_type) {
    CheckIsDisposed();
    blend_mode_ = blend_type;
  }

  renderer::GLBlendType GetBlendType() const {
    CheckIsDisposed();
    return blend_mode_;
  }

 private:
  void OnObjectDisposed() override;
  std::string_view DisposedObjectName() const override { return "Geometry"; }
  void InitDrawableData() override;
  void BeforeComposite() override;
  void Composite() override;
  void CheckDisposed() const override { CheckIsDisposed(); }

  void SetPositionInternal(size_t index, const base::Vec4 position);
  void SetTexcoordInternal(size_t index, const base::Vec2 texcoord);
  void SetColorInternal(size_t index, const base::Vec4 color);

  scoped_refptr<Bitmap> bitmap_;
  renderer::GLBlendType blend_mode_ = renderer::GLBlendType::Normal;

  renderer::VertexArray<renderer::GeometryVertex> vao_;
  std::vector<renderer::GeometryVertex> triangle_vertices_;
  size_t triangle_count_ = 0;
  size_t vbo_size_ = 0;

  bool buffer_need_update_ = false;
};

}  // namespace content

#endif  //! CONTENT_PUBLIC_GEOMETRY_H_
