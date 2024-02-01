// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "binding/mri/init_window2.h"

#include "binding/mri/mri_template.h"
#include "content/public/window2.h"

namespace binding {

MRI_DEFINE_DATATYPE_REF(Window2, "Window", content::Window2);

void InitWindow2Binding() {
  VALUE klass = rb_define_class("Window", rb_cObject);
  rb_define_alloc_func(klass, MriClassAllocate<&kWindow2DataType>);

  MriInitViewportChildBinding<content::Window2>(klass);
  MriInitDisposableBinding<content::Window2>(klass);
}

}  // namespace binding
