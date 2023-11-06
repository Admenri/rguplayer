// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_SCRIPT_PLANE_H_
#define CONTENT_SCRIPT_PLANE_H_

#include "base/math/math.h"
#include "content/script/bitmap.h"
#include "content/script/disposable.h"
#include "content/script/viewport.h"
#include "gpu/gles2/draw/quad_array.h"
#include "gpu/gles2/draw/quad_drawable.h"

namespace content {

class Plane : public base::RefCounted<Plane>,
              public Disposable,
              public ViewportChild {
 public:
  Plane(scoped_refptr<Viewport> viewport = nullptr);
  ~Plane() override;

  Plane(const Plane&) = delete;
  Plane& operator=(const Plane&) = delete;

  void SetBitmap(scoped_refptr<Bitmap> bitmap);
  scoped_refptr<Bitmap> GetBitmap() const {
    CheckIsDisposed();

    return bitmap_;
  }

  void SetOX(int ox);
  int GetOX() const {
    CheckIsDisposed();

    return ox_;
  }

  void SetOY(int oy);
  int GetOY() const {
    CheckIsDisposed();

    return oy_;
  }

  void SetZoomX(double zoom_x);
  double GetZoomX() const {
    CheckIsDisposed();

    return zoom_x_;
  }

  void SetZoomY(double zoom_y);
  double GetZoomY() const {
    CheckIsDisposed();

    return zoom_y_;
  }

  void SetOpacity(int opacity) {
    CheckIsDisposed();

    opacity_ = opacity;
  }

  int GetOpacity() const {
    CheckIsDisposed();

    return opacity_;
  }

  void SetBlendType(gpu::GLBlendType blend_type) {
    CheckIsDisposed();

    blend_type_ = blend_type;
  }

  gpu::GLBlendType GetBlendType() const {
    CheckIsDisposed();

    return blend_type_;
  }

  void SetColor(scoped_refptr<Color> color) {
    CheckIsDisposed();

    color_ = color;
  }

  scoped_refptr<Color> GetColor() const {
    CheckIsDisposed();

    return color_;
  }

  void SetTone(scoped_refptr<Tone> tone) {
    CheckIsDisposed();

    tone_ = tone;
  }

  scoped_refptr<Tone> GetTone() const {
    CheckIsDisposed();

    return tone_;
  }

 private:
  void OnObjectDisposed() override;
  std::string_view DisposedObjectName() const override { return "Plane"; }

  void BeforeComposite() override;
  void Composite() override;
  void CheckDisposed() const { CheckIsDisposed(); }
  void OnViewportRectChanged(const DrawableParent::ViewportInfo& rect) override;

  void InitPlaneInternal();
  void UpdateQuadArray();

  scoped_refptr<Bitmap> bitmap_;
  int ox_ = 0, oy_ = 0;
  double zoom_x_ = 1.0f, zoom_y_ = 1.0f;
  int opacity_ = 255;
  gpu::GLBlendType blend_type_ = gpu::GLBlendType::Normal;

  scoped_refptr<Color> color_;
  scoped_refptr<Tone> tone_;

  std::unique_ptr<gpu::QuadDrawableArray<gpu::CommonVertex>> quad_array_;
  bool quad_array_dirty_ = false;

  base::WeakPtrFactory<Plane> weak_ptr_factory_{this};
};

}  // namespace content

#endif  // CONTENT_SCRIPT_PLANE_H_