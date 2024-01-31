// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "binding/mri/init_utility.h"

#include "binding/mri/mri_util.h"
#include "content/public/utility.h"

namespace binding {

MRI_DEFINE_DATATYPE_REF(Rect, "Rect", content::Rect);

MRI_METHOD(rect_initialize) {
  scoped_refptr<content::Rect> obj;

  if (argc) {
    int x, y, w, h;
    MriParseArgsTo(argc, argv, "iiii", &x, &y, &w, &h);

    obj = new content::Rect(base::Rect(x, y, w, h));
  } else {
    obj = new content::Rect();
  }

  obj->AddRef();
  MriSetStructData(self, obj.get());

  return self;
}

MRI_METHOD(rect_initialize_copy) {
  VALUE other;
  MriParseArgsTo(argc, argv, "o", &other);

  scoped_refptr<content::Rect> other_obj =
      MriGetStructData<content::Rect>(other);

  scoped_refptr<content::Rect> obj = new content::Rect(*other_obj);
  obj->AddRef();
  MriSetStructData(self, obj.get());

  return self;
}

MRI_METHOD(rect_marshal_load) {
  std::string data;
  MriParseArgsTo(argc, argv, "s", &data);

  VALUE obj = rb_obj_alloc(self);

  scoped_refptr<content::Rect> ptr;
  MRI_GUARD(ptr = content::Rect::Deserialize(data););
  ptr->AddRef();
  MriSetStructData(obj, ptr.get());

  return obj;
}

MRI_METHOD(rect_marshal_save) {
  scoped_refptr<content::Rect> obj = MriGetStructData<content::Rect>(self);

  std::string data;
  MRI_GUARD(data = obj->Serialize();)

  return rb_str_new(data.c_str(), data.size());
}

MRI_METHOD(rect_set) {
  scoped_refptr<content::Rect> obj = MriGetStructData<content::Rect>(self);

  if (argc == 1) {
    VALUE other;
    MriParseArgsTo(argc, argv, "o", &other);
    scoped_refptr<content::Rect> other_obj =
        MriCheckStructData<content::Rect>(other, kRectDataType);

    obj->Set(other_obj);
  } else {
    int x, y, w, h;
    MriParseArgsTo(argc, argv, "iiii", &x, &y, &w, &h);

    obj->Set(base::Rect(x, y, w, h));
  }

  return self;
}

MRI_METHOD(rect_empty) {
  scoped_refptr<content::Rect> obj = MriGetStructData<content::Rect>(self);

  obj->Empty();

  return self;
}

#define RECT_ATTR_DEFINE(name)                                                \
  MRI_METHOD(rect_attr_get_##name) {                                          \
    scoped_refptr<content::Rect> obj = MriGetStructData<content::Rect>(self); \
    return rb_fix_new(obj->Get##name());                                      \
  }                                                                           \
  MRI_METHOD(rect_attr_set_##name) {                                          \
    scoped_refptr<content::Rect> obj = MriGetStructData<content::Rect>(self); \
    int v;                                                                    \
    MriParseArgsTo(argc, argv, "i", &v);                                      \
    obj->Set##name(v);                                                        \
    return self;                                                              \
  }

RECT_ATTR_DEFINE(X);
RECT_ATTR_DEFINE(Y);
RECT_ATTR_DEFINE(Width);
RECT_ATTR_DEFINE(Height);

MRI_METHOD(rect_stringify) {
  scoped_refptr<content::Rect> obj = MriGetStructData<content::Rect>(self);

  return rb_sprintf("(%d, %d, %d, %d)", obj->GetX(), obj->GetY(),
                    obj->GetWidth(), obj->GetHeight());
}

MRI_METHOD(rect_equal) {
  scoped_refptr<content::Rect> obj = MriGetStructData<content::Rect>(self);

  VALUE other;
  MriParseArgsTo(argc, argv, "o", &other);

  if (!rb_typeddata_is_kind_of(other, &kRectDataType))
    return Qfalse;

  scoped_refptr<content::Rect> other_obj =
      MriCheckStructData<content::Rect>(other, kRectDataType);

  return (*other_obj == *obj) ? Qtrue : Qfalse;
}

void InitRectBinding() {
  VALUE klass = rb_define_class("Rect", rb_cObject);
  rb_define_alloc_func(klass, MriClassAllocate<&kRectDataType>);

  MriDefineMethod(klass, "initialize", rect_initialize);
  MriDefineMethod(klass, "initialize_copy", rect_initialize_copy);
  MriDefineClassMethod(klass, "_load", rect_marshal_load);
  MriDefineMethod(klass, "_save", rect_marshal_save);

  MriDefineMethod(klass, "x", rect_attr_get_X);
  MriDefineMethod(klass, "x=", rect_attr_set_X);
  MriDefineMethod(klass, "y", rect_attr_get_Y);
  MriDefineMethod(klass, "y=", rect_attr_set_Y);
  MriDefineMethod(klass, "width", rect_attr_get_Width);
  MriDefineMethod(klass, "width=", rect_attr_set_Width);
  MriDefineMethod(klass, "height", rect_attr_get_Height);
  MriDefineMethod(klass, "height=", rect_attr_set_Height);

  MriDefineMethod(klass, "set", rect_set);
  MriDefineMethod(klass, "empty", rect_empty);
  MriDefineMethod(klass, "inspect", rect_stringify);
  MriDefineMethod(klass, "to_s", rect_stringify);
  MriDefineMethod(klass, "==", rect_equal);
  MriDefineMethod(klass, "===", rect_equal);
  MriDefineMethod(klass, "eql?", rect_equal);
}

MRI_DEFINE_DATATYPE_REF(Color, "Color", content::Color);

void InitColorBinding() {
  VALUE klass = rb_define_class("Color", rb_cObject);
  rb_define_alloc_func(klass, MriClassAllocate<&kColorDataType>);
}

void InitUtilityBinding() {
  InitRectBinding();
  InitColorBinding();
}

}  // namespace binding
