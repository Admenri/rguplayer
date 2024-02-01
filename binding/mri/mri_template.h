// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef BINDING_MRI_MRI_TEMPLATE_H_
#define BINDING_MRI_MRI_TEMPLATE_H_

#include "content/common/serializable.h"
#include "content/public/disposable.h"
#include "content/public/drawable.h"
#include "content/public/flashable.h"
#include "content/public/viewport.h"

namespace binding {

MRI_DECLARE_DATATYPE(Color);
MRI_DECLARE_DATATYPE(Viewport);

template <typename Ty>
MRI_METHOD(serializable_marshal_load) {
  std::string data;
  MriParseArgsTo(argc, argv, "s", &data);

  VALUE obj = rb_obj_alloc(self);

  scoped_refptr<Ty> ptr;
  MRI_GUARD(ptr = Ty::Deserialize(data););
  ptr->AddRef();
  MriSetStructData(obj, ptr.get());

  return obj;
}

template <typename Ty>
MRI_METHOD(serializable_marshal_dump) {
  content::Serializable* obj = MriGetStructData<Ty>(self);

  std::string data;
  MRI_GUARD(data = obj->Serialize();)

  return rb_str_new(data.c_str(), data.size());
}

template <typename Ty>
void MriInitSerializableBinding(VALUE klass) {
  MriDefineClassMethod(klass, "_load", serializable_marshal_load<Ty>);
  MriDefineMethod(klass, "_dump", serializable_marshal_dump<Ty>);
}

template <typename Ty>
MRI_METHOD(disposable_dispose) {
  content::Disposable* obj = MriGetStructData<Ty>(self);

  if (!obj || obj->IsDisposed())
    return Qnil;

  obj->Dispose();

  return Qnil;
}

template <typename Ty>
MRI_METHOD(disposable_is_disposed) {
  content::Disposable* obj = MriGetStructData<Ty>(self);

  if (!obj)
    return Qtrue;

  return obj->IsDisposed() ? Qtrue : Qfalse;
}

template <typename Ty>
void MriInitDisposableBinding(VALUE klass) {
  MriDefineMethod(klass, "dispose", disposable_dispose<Ty>);
  MriDefineMethod(klass, "disposed?", disposable_is_disposed<Ty>);
}

template <typename Ty>
MRI_METHOD(flashable_flash) {
  content::Flashable* obj = MriGetStructData<Ty>(self);

  VALUE color;
  int duration;
  MriParseArgsTo(argc, argv, "oi", &color, &duration);

  if (NIL_P(color)) {
    obj->Flash(nullptr, duration);
    return Qnil;
  }

  scoped_refptr<content::Color> color_obj =
      MriCheckStructData<content::Color>(color, kColorDataType);

  obj->Flash(color_obj, duration);

  return Qnil;
}

template <typename Ty>
MRI_METHOD(flashable_update) {
  content::Flashable* obj = MriGetStructData<Ty>(self);

  obj->Update();

  return Qnil;
}

template <typename Ty>
void MriInitFlashableBinding(VALUE klass) {
  MriDefineMethod(klass, "flash", flashable_flash<Ty>);
  MriDefineMethod(klass, "update", flashable_update<Ty>);
}

template <typename Ty>
MRI_METHOD(drawable_get_z) {
  content::Drawable* obj = MriGetStructData<Ty>(self);
  int v = 0;
  MRI_GUARD(v = obj->GetZ(););
  return rb_fix_new(v);
}

template <typename Ty>
MRI_METHOD(drawable_set_z) {
  content::Drawable* obj = MriGetStructData<Ty>(self);
  int v = 0;
  MriParseArgsTo(argc, argv, "i", &v);
  MRI_GUARD(obj->SetZ(v););
  return rb_fix_new(v);
}

template <typename Ty>
MRI_METHOD(drawable_get_visible) {
  content::Drawable* obj = MriGetStructData<Ty>(self);
  bool v = false;
  MRI_GUARD(v = obj->GetVisible(););
  return v ? Qtrue : Qfalse;
}

template <typename Ty>
MRI_METHOD(drawable_set_visible) {
  content::Drawable* obj = MriGetStructData<Ty>(self);
  bool v = false;
  MriParseArgsTo(argc, argv, "b", &v);
  MRI_GUARD(obj->SetVisible(v););
  return v ? Qtrue : Qfalse;
}

template <typename Ty>
void MriInitDrawableBinding(VALUE klass) {
  MriDefineMethod(klass, "z", drawable_get_z<Ty>);
  MriDefineMethod(klass, "z=", drawable_set_z<Ty>);
  MriDefineMethod(klass, "visible", drawable_get_visible<Ty>);
  MriDefineMethod(klass, "visible=", drawable_set_visible<Ty>);
}

template <typename Ty>
MRI_METHOD(viewportchild_get_viewport) {
  content::ViewportChild* obj = MriGetStructData<Ty>(self);

  MRI_GUARD(obj->CheckIsDisposed(););

  return rb_iv_get(self, "_viewport");
}

template <typename Ty>
MRI_METHOD(viewportchild_set_viewport) {
  content::ViewportChild* obj = MriGetStructData<Ty>(self);

  VALUE v;
  MriParseArgsTo(argc, argv, "o", &v);

  scoped_refptr<content::Viewport> viewport;
  if (!NIL_P(v))
    viewport = MriCheckStructData<content::Viewport>(v, kViewportDataType);

  MRI_GUARD(obj->SetViewport(viewport););

  rb_iv_set(self, "_viewport", v);

  return v ? Qtrue : Qfalse;
}

template <typename Ty>
scoped_refptr<Ty> MriInitializeViewportchild(
    scoped_refptr<content::Graphics> screen,
    int argc,
    VALUE* argv,
    VALUE self) {
  VALUE v;
  MriParseArgsTo(argc, argv, "|o", &v);

  scoped_refptr<content::Viewport> viewport;
  if (!NIL_P(v))
    viewport = MriCheckStructData<content::Viewport>(v, kViewportDataType);

  scoped_refptr<Ty> obj;
  MRI_GUARD(obj = new Ty(screen, viewport););
  rb_iv_set(self, "_viewport", v);

  return obj;
}

template <typename Ty>
void MriInitViewportChildBinding(VALUE klass) {
  MriInitDrawableBinding<Ty>(klass);
  MriDefineMethod(klass, "viewport", viewportchild_set_viewport<Ty>);
  MriDefineMethod(klass, "viewport=", drawable_set_visible<Ty>);
}

}  // namespace binding

#endif  //! BINDING_MRI_MRI_TEMPLATE_H_
