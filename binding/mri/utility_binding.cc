// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "binding/mri/binding_util.h"

#include "content/public/utility.h"

namespace binding {

DEF_TYPE(Rect, "Rect", content::Rect);

MRI_METHOD(rect_initialize) {
  scoped_refptr<content::Rect> obj;

  if (!argc) {
    obj = new content::Rect();
  } else {
    int x, y, z, w;
    GetArgsFrom(argc, argv, "iiii", &x, &y, &z, &w);
    obj = new content::Rect(base::Rect(x, y, z, w));
  }

  SetPrivateData<content::Rect>(self, obj);
  return self;
}

MRI_METHOD(rect_initialize_dup) {
  VALUE dup_obj;
  GetArgsFrom(argc, argv, "o", &dup_obj);
  if (!OBJ_INIT_COPY(self, dup_obj))
    return self;

  scoped_refptr<content::Rect> dup_rt = GetPrivateData<content::Rect>(dup_obj);

  scoped_refptr<content::Rect> obj = new content::Rect(*dup_rt);
  SetPrivateData<content::Rect>(self, obj);
  return self;
}

MRI_METHOD(rect_marshal_load) {
  char* data_ptr;
  int data_size;
  GetArgsFrom(argc, argv, "s", &data_ptr, &data_size);

  scoped_refptr<content::Rect> obj =
      content::Rect::Deserialize(std::string(data_ptr, data_size));

  VALUE rb_obj = rb_obj_alloc(self);
  SetPrivateData(rb_obj, obj);

  return rb_obj;
}

MRI_METHOD(rect_marshal_dump) {
  scoped_refptr<content::Rect> obj = GetPrivateData<content::Rect>(self);

  std::string data = obj->Serialize();

  return rb_str_new(data.c_str(), data.size());
}

MRI_METHOD(rect_set) {
  scoped_refptr<content::Rect> obj = GetPrivateData<content::Rect>(self);

  if (argc == 1) {
    scoped_refptr<content::Rect> other =
        GetPrivateDataCheck<content::Rect>(argv[0], RectType);
    obj->Set(other);
  } else {
    int x, y, w, h;
    GetArgsFrom(argc, argv, "iiii", &x, &y, &w, &h);
    obj->Set(base::Rect(x, y, w, h));
  }

  std::string data = obj->Serialize();

  return self;
}

MRI_METHOD(rect_empty) {
  scoped_refptr<content::Rect> obj = GetPrivateData<content::Rect>(self);
  obj->Empty();
  return self;
}

MRI_METHOD(rect_get_x) {
  scoped_refptr<content::Rect> obj = GetPrivateData<content::Rect>(self);
  return rb_fix_new(obj->GetX());
}

MRI_METHOD(rect_set_x) {
  scoped_refptr<content::Rect> obj = GetPrivateData<content::Rect>(self);

  int v;
  GetArgsFrom(argc, argv, "i", &v);

  obj->SetX(v);

  return self;
}

MRI_METHOD(rect_get_y) {
  scoped_refptr<content::Rect> obj = GetPrivateData<content::Rect>(self);
  return rb_fix_new(obj->GetY());
}

MRI_METHOD(rect_set_y) {
  scoped_refptr<content::Rect> obj = GetPrivateData<content::Rect>(self);

  int v;
  GetArgsFrom(argc, argv, "i", &v);

  obj->SetY(v);

  return self;
}

MRI_METHOD(rect_get_width) {
  scoped_refptr<content::Rect> obj = GetPrivateData<content::Rect>(self);
  return rb_fix_new(obj->GetWidth());
}

MRI_METHOD(rect_set_width) {
  scoped_refptr<content::Rect> obj = GetPrivateData<content::Rect>(self);

  int v;
  GetArgsFrom(argc, argv, "i", &v);

  obj->SetWidth(v);

  return self;
}

MRI_METHOD(rect_get_height) {
  scoped_refptr<content::Rect> obj = GetPrivateData<content::Rect>(self);
  return rb_fix_new(obj->GetHeight());
}

MRI_METHOD(rect_set_height) {
  scoped_refptr<content::Rect> obj = GetPrivateData<content::Rect>(self);

  int v;
  GetArgsFrom(argc, argv, "i", &v);

  obj->SetHeight(v);

  return self;
}

MRI_METHOD(rect_equal) {
  scoped_refptr<content::Rect> obj = GetPrivateData<content::Rect>(self);

  VALUE o;
  GetArgsFrom(argc, argv, "o", &o);
  if (!rb_typeddata_is_kind_of(o, &RectType))
    return Qfalse;

  scoped_refptr<content::Rect> other =
      GetPrivateDataCheck<content::Rect>(o, RectType);

  return (*other == *obj) ? Qtrue : Qfalse;
}

MRI_METHOD(rect_stringify) {
  scoped_refptr<content::Rect> obj = GetPrivateData<content::Rect>(self);

  return rb_sprintf("(%d, %d, %d, %d)", obj->GetX(), obj->GetY(),
                    obj->GetWidth(), obj->GetHeight());
}

void InitRectBinding() {
  VALUE rect_obj = rb_define_class("Rect", rb_cObject);
  rb_define_alloc_func(rect_obj, ClassAllocate<&RectType>);

  DefineMethod(rect_obj, "initialize", rect_initialize);
  DefineMethod(rect_obj, "initialize_copy", rect_initialize_dup);
  DefineClassMethod(rect_obj, "_load", rect_marshal_load);
  DefineMethod(rect_obj, "_dump", rect_marshal_dump);
  DefineMethod(rect_obj, "set", rect_set);
  DefineMethod(rect_obj, "empty", rect_empty);
  DefineMethod(rect_obj, "x", rect_get_x);
  DefineMethod(rect_obj, "x=", rect_set_x);
  DefineMethod(rect_obj, "y", rect_get_y);
  DefineMethod(rect_obj, "y=", rect_set_y);
  DefineMethod(rect_obj, "width", rect_get_width);
  DefineMethod(rect_obj, "width=", rect_set_width);
  DefineMethod(rect_obj, "height", rect_get_height);
  DefineMethod(rect_obj, "height=", rect_set_height);
  DefineMethod(rect_obj, "to_s", rect_stringify);
  DefineMethod(rect_obj, "inspect", rect_stringify);
  DefineMethod(rect_obj, "==", rect_equal);
  DefineMethod(rect_obj, "===", rect_equal);
  DefineMethod(rect_obj, "eql?", rect_equal);
}

DEF_TYPE(Color, "Color", content::Color);

MRI_METHOD(color_initialize) {
  scoped_refptr<content::Color> obj;

  if (!argc) {
    obj = new content::Color();
  } else {
    int x, y, z, w = 255;
    GetArgsFrom(argc, argv, "iii|i", &x, &y, &z, &w);
    obj = new content::Color(x, y, z, w);
  }

  SetPrivateData<content::Color>(self, obj);
  return self;
}

MRI_METHOD(color_initialize_dup) {
  VALUE dup_obj;
  GetArgsFrom(argc, argv, "o", &dup_obj);
  if (!OBJ_INIT_COPY(self, dup_obj))
    return self;

  scoped_refptr<content::Color> dup_rt =
      GetPrivateData<content::Color>(dup_obj);

  scoped_refptr<content::Color> obj = new content::Color(*dup_rt);
  SetPrivateData<content::Color>(self, obj);
  return self;
}

MRI_METHOD(color_marshal_load) {
  char* data_ptr;
  int data_size;
  GetArgsFrom(argc, argv, "s", &data_ptr, &data_size);

  scoped_refptr<content::Color> obj =
      content::Color::Deserialize(std::string(data_ptr, data_size));

  VALUE rb_obj = rb_obj_alloc(self);
  SetPrivateData(rb_obj, obj);

  return rb_obj;
}

MRI_METHOD(color_marshal_dump) {
  scoped_refptr<content::Color> obj = GetPrivateData<content::Color>(self);

  std::string data = obj->Serialize();

  return rb_str_new(data.c_str(), data.size());
}

MRI_METHOD(color_set) {
  scoped_refptr<content::Color> obj = GetPrivateData<content::Color>(self);

  if (argc == 1) {
    scoped_refptr<content::Color> other =
        GetPrivateDataCheck<content::Color>(argv[0], ColorType);
    obj->Set(other);
  } else {
    int x, y, z, w = 255;
    GetArgsFrom(argc, argv, "iii|i", &x, &y, &z, &w);
    obj->Set(x, y, z, w);
  }

  std::string data = obj->Serialize();

  return self;
}

MRI_METHOD(color_get_red) {
  scoped_refptr<content::Color> obj = GetPrivateData<content::Color>(self);
  return rb_fix_new(obj->GetRed());
}

MRI_METHOD(color_set_red) {
  scoped_refptr<content::Color> obj = GetPrivateData<content::Color>(self);

  int v;
  GetArgsFrom(argc, argv, "i", &v);

  obj->SetRed(v);

  return self;
}

MRI_METHOD(color_get_green) {
  scoped_refptr<content::Color> obj = GetPrivateData<content::Color>(self);
  return rb_fix_new(obj->GetGreen());
}

MRI_METHOD(color_set_green) {
  scoped_refptr<content::Color> obj = GetPrivateData<content::Color>(self);

  int v;
  GetArgsFrom(argc, argv, "i", &v);

  obj->SetGreen(v);

  return self;
}

MRI_METHOD(color_get_blue) {
  scoped_refptr<content::Color> obj = GetPrivateData<content::Color>(self);
  return rb_fix_new(obj->GetBlue());
}

MRI_METHOD(color_set_blue) {
  scoped_refptr<content::Color> obj = GetPrivateData<content::Color>(self);

  int v;
  GetArgsFrom(argc, argv, "i", &v);

  obj->SetBlue(v);

  return self;
}

MRI_METHOD(color_get_alpha) {
  scoped_refptr<content::Color> obj = GetPrivateData<content::Color>(self);
  return rb_fix_new(obj->GetAlpha());
}

MRI_METHOD(color_set_alpha) {
  scoped_refptr<content::Color> obj = GetPrivateData<content::Color>(self);

  int v;
  GetArgsFrom(argc, argv, "i", &v);

  obj->SetAlpha(v);

  return self;
}

MRI_METHOD(color_equal) {
  scoped_refptr<content::Color> obj = GetPrivateData<content::Color>(self);

  VALUE o;
  GetArgsFrom(argc, argv, "o", &o);
  if (!rb_typeddata_is_kind_of(o, &ColorType))
    return Qfalse;

  scoped_refptr<content::Color> other =
      GetPrivateDataCheck<content::Color>(o, ColorType);

  return (*other == *obj) ? Qtrue : Qfalse;
}

MRI_METHOD(color_stringify) {
  scoped_refptr<content::Color> obj = GetPrivateData<content::Color>(self);

  return rb_sprintf("(%d, %d, %d, %d)", obj->GetRed(), obj->GetGreen(),
                    obj->GetBlue(), obj->GetAlpha());
}

void InitColorBinding() {
  VALUE color_obj = rb_define_class("Color", rb_cObject);
  rb_define_alloc_func(color_obj, ClassAllocate<&ColorType>);

  DefineMethod(color_obj, "initialize", color_initialize);
  DefineMethod(color_obj, "initialize_copy", color_initialize_dup);
  DefineClassMethod(color_obj, "_load", color_marshal_load);
  DefineMethod(color_obj, "_dump", color_marshal_dump);
  DefineMethod(color_obj, "set", color_set);
  DefineMethod(color_obj, "red", color_get_red);
  DefineMethod(color_obj, "red=", color_set_red);
  DefineMethod(color_obj, "green", color_get_green);
  DefineMethod(color_obj, "green=", color_set_green);
  DefineMethod(color_obj, "blue", color_get_blue);
  DefineMethod(color_obj, "blue=", color_set_blue);
  DefineMethod(color_obj, "alpha", color_get_alpha);
  DefineMethod(color_obj, "alpha=", color_set_alpha);
  DefineMethod(color_obj, "to_s", color_stringify);
  DefineMethod(color_obj, "inspect", color_stringify);
  DefineMethod(color_obj, "==", color_equal);
  DefineMethod(color_obj, "===", color_equal);
  DefineMethod(color_obj, "eql?", color_equal);
}

DEF_TYPE(Tone, "Tone", content::Tone);

MRI_METHOD(tone_initialize) {
  scoped_refptr<content::Tone> obj;

  if (!argc) {
    obj = new content::Tone();
  } else {
    int x, y, z, w = 0;
    GetArgsFrom(argc, argv, "iii|i", &x, &y, &z, &w);
    obj = new content::Tone(x, y, z, w);
  }

  SetPrivateData<content::Tone>(self, obj);
  return self;
}

MRI_METHOD(tone_initialize_dup) {
  VALUE dup_obj;
  GetArgsFrom(argc, argv, "o", &dup_obj);
  if (!OBJ_INIT_COPY(self, dup_obj))
    return self;

  scoped_refptr<content::Tone> dup_rt = GetPrivateData<content::Tone>(dup_obj);

  scoped_refptr<content::Tone> obj = new content::Tone(*dup_rt);
  SetPrivateData<content::Tone>(self, obj);
  return self;
}

MRI_METHOD(tone_marshal_load) {
  char* data_ptr;
  int data_size;
  GetArgsFrom(argc, argv, "s", &data_ptr, &data_size);

  scoped_refptr<content::Tone> obj =
      content::Tone::Deserialize(std::string(data_ptr, data_size));

  VALUE rb_obj = rb_obj_alloc(self);
  SetPrivateData(rb_obj, obj);

  return rb_obj;
}

MRI_METHOD(tone_marshal_dump) {
  scoped_refptr<content::Tone> obj = GetPrivateData<content::Tone>(self);

  std::string data = obj->Serialize();

  return rb_str_new(data.c_str(), data.size());
}

MRI_METHOD(tone_set) {
  scoped_refptr<content::Tone> obj = GetPrivateData<content::Tone>(self);

  if (argc == 1) {
    scoped_refptr<content::Tone> other =
        GetPrivateDataCheck<content::Tone>(argv[0], ToneType);
    obj->Set(other);
  } else {
    int x, y, z, w = 255;
    GetArgsFrom(argc, argv, "iii|i", &x, &y, &z, &w);
    obj->Set(x, y, z, w);
  }

  std::string data = obj->Serialize();

  return self;
}

MRI_METHOD(tone_get_red) {
  scoped_refptr<content::Tone> obj = GetPrivateData<content::Tone>(self);
  return rb_fix_new(obj->GetRed());
}

MRI_METHOD(tone_set_red) {
  scoped_refptr<content::Tone> obj = GetPrivateData<content::Tone>(self);

  int v;
  GetArgsFrom(argc, argv, "i", &v);

  obj->SetRed(v);

  return self;
}

MRI_METHOD(tone_get_green) {
  scoped_refptr<content::Tone> obj = GetPrivateData<content::Tone>(self);
  return rb_fix_new(obj->GetGreen());
}

MRI_METHOD(tone_set_green) {
  scoped_refptr<content::Tone> obj = GetPrivateData<content::Tone>(self);

  int v;
  GetArgsFrom(argc, argv, "i", &v);

  obj->SetGreen(v);

  return self;
}

MRI_METHOD(tone_get_blue) {
  scoped_refptr<content::Tone> obj = GetPrivateData<content::Tone>(self);
  return rb_fix_new(obj->GetBlue());
}

MRI_METHOD(tone_set_blue) {
  scoped_refptr<content::Tone> obj = GetPrivateData<content::Tone>(self);

  int v;
  GetArgsFrom(argc, argv, "i", &v);

  obj->SetBlue(v);

  return self;
}

MRI_METHOD(tone_get_gray) {
  scoped_refptr<content::Tone> obj = GetPrivateData<content::Tone>(self);
  return rb_fix_new(obj->GetGray());
}

MRI_METHOD(tone_set_gray) {
  scoped_refptr<content::Tone> obj = GetPrivateData<content::Tone>(self);

  int v;
  GetArgsFrom(argc, argv, "i", &v);

  obj->SetGray(v);

  return self;
}

MRI_METHOD(tone_equal) {
  scoped_refptr<content::Tone> obj = GetPrivateData<content::Tone>(self);

  VALUE o;
  GetArgsFrom(argc, argv, "o", &o);
  if (!rb_typeddata_is_kind_of(o, &ToneType))
    return Qfalse;

  scoped_refptr<content::Tone> other =
      GetPrivateDataCheck<content::Tone>(o, ToneType);

  return (*other == *obj) ? Qtrue : Qfalse;
}

MRI_METHOD(tone_stringify) {
  scoped_refptr<content::Tone> obj = GetPrivateData<content::Tone>(self);

  return rb_sprintf("(%d, %d, %d, %d)", obj->GetRed(), obj->GetGreen(),
                    obj->GetBlue(), obj->GetGray());
}

void InitToneBinding() {
  VALUE tone_obj = rb_define_class("Tone", rb_cObject);
  rb_define_alloc_func(tone_obj, ClassAllocate<&ToneType>);

  DefineMethod(tone_obj, "initialize", tone_initialize);
  DefineMethod(tone_obj, "initialize_copy", tone_initialize_dup);
  DefineClassMethod(tone_obj, "_load", tone_marshal_load);
  DefineMethod(tone_obj, "_dump", tone_marshal_dump);
  DefineMethod(tone_obj, "set", tone_set);
  DefineMethod(tone_obj, "red", tone_get_red);
  DefineMethod(tone_obj, "red=", tone_set_red);
  DefineMethod(tone_obj, "green", tone_get_green);
  DefineMethod(tone_obj, "green=", tone_set_green);
  DefineMethod(tone_obj, "blue", tone_get_blue);
  DefineMethod(tone_obj, "blue=", tone_set_blue);
  DefineMethod(tone_obj, "gray", tone_get_gray);
  DefineMethod(tone_obj, "gray=", tone_set_gray);
  DefineMethod(tone_obj, "to_s", tone_stringify);
  DefineMethod(tone_obj, "inspect", tone_stringify);
  DefineMethod(tone_obj, "==", tone_equal);
  DefineMethod(tone_obj, "===", tone_equal);
  DefineMethod(tone_obj, "eql?", tone_equal);
}

void InitUtilityBinding() {
  InitRectBinding();
  InitColorBinding();
  InitToneBinding();
}

}  // namespace binding
