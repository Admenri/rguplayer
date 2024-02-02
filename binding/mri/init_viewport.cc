// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "binding/mri/init_viewport.h"

#include "binding/mri/init_utility.h"
#include "binding/mri/mri_template.h"
#include "content/public/viewport.h"

namespace binding {

MRI_DEFINE_DATATYPE_REF(Viewport, "Viewport", content::Viewport);

MRI_METHOD(viewport_initialize) {
  scoped_refptr<content::Graphics> screen = MriGetGlobalRunner()->graphics();
  scoped_refptr<content::Viewport> obj;

  if (argc == 0) {
    obj = new content::Viewport(screen);
  } else if (argc == 1) {
    VALUE rect;
    MriParseArgsTo(argc, argv, "o", &rect);

    scoped_refptr<content::Rect> rect_obj =
        MriCheckStructData<content::Rect>(rect, kRectDataType);

    obj = new content::Viewport(screen, rect_obj->AsBase());
  } else {
    int x, y, width, height;

    MriParseArgsTo(argc, argv, "iiii", &x, &y, &width, &height);

    obj = new content::Viewport(screen, base::Rect(x, y, width, height));
  }

  obj->AddRef();
  MriSetStructData<content::Viewport>(self, obj.get());

  MriWrapProperty(self, obj->GetRect(), "_rect", kRectDataType);
  MriWrapProperty(self, obj->GetColor(), "_color", kColorDataType);
  MriWrapProperty(self, obj->GetTone(), "_tone", kToneDataType);

  return self;
}

#define VIEWPORT_DEFINE_INT_ATTR(name)             \
  MRI_METHOD(viewport_get_##name) {                \
    scoped_refptr<content::Viewport> obj =         \
        MriGetStructData<content::Viewport>(self); \
    int v = 0;                                     \
    MRI_GUARD(v = obj->Get##name(););              \
    return rb_fix_new(v);                          \
  }                                                \
  MRI_METHOD(viewport_set_##name) {                \
    scoped_refptr<content::Viewport> obj =         \
        MriGetStructData<content::Viewport>(self); \
    int v = 0;                                     \
    MriParseArgsTo(argc, argv, "i", &v);           \
    MRI_GUARD(obj->Set##name(v););                 \
    return rb_fix_new(v);                          \
  }

VIEWPORT_DEFINE_INT_ATTR(OX);
VIEWPORT_DEFINE_INT_ATTR(OY);

#define VIEWPORT_DEFINE_VAL_ATTR(name, ivname)                           \
  MRI_METHOD(viewport_get_##name) {                                      \
    scoped_refptr<content::Viewport> obj =                               \
        MriGetStructData<content::Viewport>(self);                       \
    MRI_GUARD(obj->CheckIsDisposed(););                                  \
    return rb_iv_get(self, #ivname);                                     \
  }                                                                      \
  MRI_METHOD(viewport_set_##name) {                                      \
    MriCheckArgc(argc, 1);                                               \
    scoped_refptr<content::Viewport> obj =                               \
        MriGetStructData<content::Viewport>(self);                       \
    VALUE propObj = *argv;                                               \
    scoped_refptr<content::##name> prop =                                \
        MriCheckStructData<content::##name>(propObj, k##name##DataType); \
    MRI_GUARD(obj->Set##name(prop);)                                     \
    return propObj;                                                      \
  }

VIEWPORT_DEFINE_VAL_ATTR(Rect, _rect);
VIEWPORT_DEFINE_VAL_ATTR(Color, _color);
VIEWPORT_DEFINE_VAL_ATTR(Tone, _tone);

void InitViewportBinding() {
  VALUE klass = rb_define_class("Viewport", rb_cObject);
  rb_define_alloc_func(klass, MriClassAllocate<&kViewportDataType>);

  MriInitDisposableBinding<content::Viewport>(klass);
  MriInitFlashableBinding<content::Viewport>(klass);
  MriInitDrawableBinding<content::Viewport>(klass);

  MriDefineMethod(klass, "initialize", viewport_initialize);

  MriDefineAttr(klass, "ox", viewport, OX);
  MriDefineAttr(klass, "oy", viewport, OY);
  MriDefineAttr(klass, "rect", viewport, Rect);
  MriDefineAttr(klass, "color", viewport, Color);
  MriDefineAttr(klass, "tone", viewport, Tone);
}

}  // namespace binding
