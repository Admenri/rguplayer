// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "binding/mri/init_geometry.h"

#include "binding/mri/init_bitmap.h"
#include "binding/mri/init_utility.h"
#include "binding/mri/mri_template.h"
#include "content/public/geometry.h"

namespace binding {

MRI_DEFINE_DATATYPE_REF(Geometry, "Geometry", content::Geometry);

MRI_METHOD(geometry_initialize) {
  scoped_refptr<content::Graphics> screen = MriGetGlobalRunner()->graphics();

  MriInitializeViewportchild<content::Geometry>(screen, argc, argv, self);

  return self;
}

MRI_METHOD(geometry_get_bitmap) {
  scoped_refptr<content::Geometry> obj =
      MriGetStructData<content::Geometry>(self);
  return rb_iv_get(self, "_bitmap");
}

MRI_METHOD(geometry_set_bitmap) {
  MriCheckArgc(argc, 1);
  scoped_refptr<content::Geometry> obj =
      MriGetStructData<content::Geometry>(self);
  VALUE propObj = *argv;
  scoped_refptr<content::Bitmap> prop;
  if (!NIL_P(propObj))
    prop = MriCheckStructData<content::Bitmap>(propObj, kBitmapDataType);
  MRI_GUARD(obj->SetBitmap(prop););
  rb_iv_set(self, "_bitmap", *argv);
  return propObj;
}

MRI_METHOD(geometry_get_blendtype) {
  scoped_refptr<content::Geometry> obj =
      MriGetStructData<content::Geometry>(self);
  int v;
  MRI_GUARD(v = (int)obj->GetBlendType(););
  return rb_fix_new(v);
}
MRI_METHOD(geometry_set_blendtype) {
  scoped_refptr<content::Geometry> obj =
      MriGetStructData<content::Geometry>(self);
  int v;
  MriParseArgsTo(argc, argv, "i", &v);
  MRI_GUARD(obj->SetBlendType((renderer::GLBlendType)v););
  return *argv;
}

MRI_METHOD(geometry_get_capacity) {
  scoped_refptr<content::Geometry> obj =
      MriGetStructData<content::Geometry>(self);
  size_t c;
  MRI_GUARD(c = obj->GetCapacity(););
  return UINT2NUM(c);
}

MRI_METHOD(geometry_resize) {
  scoped_refptr<content::Geometry> obj =
      MriGetStructData<content::Geometry>(self);
  int size;
  MriParseArgsTo(argc, argv, "i", &size);
  MRI_GUARD(obj->Resize(size););
  return Qnil;
}

MRI_METHOD(geometry_set_position) {
  scoped_refptr<content::Geometry> obj =
      MriGetStructData<content::Geometry>(self);
  int index, x, y;
  double z, w;
  MriParseArgsTo(argc, argv, "iiiff", &index, &x, &y, &z, &w);
  MRI_GUARD(obj->SetPosition(index, base::Vec4(x, y, z, w)););
  return Qnil;
}

MRI_METHOD(geometry_set_texcoord) {
  scoped_refptr<content::Geometry> obj =
      MriGetStructData<content::Geometry>(self);
  int index;
  double x, y;
  MriParseArgsTo(argc, argv, "iff", &index, &x, &y);
  MRI_GUARD(obj->SetTexcoord(index, base::Vec2(x, y)););
  return Qnil;
}

MRI_METHOD(geometry_set_color) {
  scoped_refptr<content::Geometry> obj =
      MriGetStructData<content::Geometry>(self);
  int index;
  VALUE o;
  MriParseArgsTo(argc, argv, "io", &index, &o);

  scoped_refptr<content::Color> color =
      MriCheckStructData<content::Color>(o, kColorDataType);
  MRI_GUARD(obj->SetColor(index, color->AsBase()););

  return Qnil;
}

void InitGeometryBinding() {
  VALUE klass = rb_define_class("Geometry", rb_cObject);
  rb_define_alloc_func(klass, MriClassAllocate<&kGeometryDataType>);

  MriInitViewportChildBinding<content::Geometry>(klass);
  MriInitDisposableBinding<content::Geometry>(klass);

  MriDefineMethod(klass, "initialize", geometry_initialize);
  MriDefineMethod(klass, "capacity", geometry_get_capacity);
  MriDefineMethod(klass, "resize", geometry_resize);
  MriDefineMethod(klass, "set_position", geometry_set_position);
  MriDefineMethod(klass, "set_texcoord", geometry_set_texcoord);
  MriDefineMethod(klass, "set_color", geometry_set_color);

  MriDefineAttr(klass, "bitmap", geometry, bitmap);
  MriDefineAttr(klass, "blend_type", geometry, blendtype);
}

}  // namespace binding
