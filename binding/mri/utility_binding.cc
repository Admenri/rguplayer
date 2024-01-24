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

void InitUtilityBinding() {
  VALUE rect_obj = rb_define_class("Rect", rb_cObject);
  rb_define_alloc_func(rect_obj, ClassAllocate<&RectType>);

  DefineMethod(rect_obj, "initialize", rect_initialize);
}

}  // namespace binding
