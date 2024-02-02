// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "binding/mri/init_input.h"

#include "content/public/input.h"

namespace binding {

MRI_METHOD(input_update) {
  scoped_refptr<content::Input> input = MriGetGlobalRunner()->input();

  input->Update();

  return Qnil;
}

MRI_METHOD(input_is_pressed) {
  scoped_refptr<content::Input> input = MriGetGlobalRunner()->input();

  ID key;
  MriParseArgsTo(argc, argv, "n", &key);

  bool v = input->IsPressed(rb_id2name(key));
  return MRI_BOOL_NEW(v);
}

MRI_METHOD(input_is_triggered) {
  scoped_refptr<content::Input> input = MriGetGlobalRunner()->input();

  ID key;
  MriParseArgsTo(argc, argv, "n", &key);

  bool v = input->IsTriggered(rb_id2name(key));
  return MRI_BOOL_NEW(v);
}

MRI_METHOD(input_is_repeated) {
  scoped_refptr<content::Input> input = MriGetGlobalRunner()->input();

  ID key;
  MriParseArgsTo(argc, argv, "n", &key);

  bool v = input->IsRepeated(rb_id2name(key));
  return MRI_BOOL_NEW(v);
}

MRI_METHOD(input_dir4) {
  scoped_refptr<content::Input> input = MriGetGlobalRunner()->input();
  return rb_fix_new(input->Dir4());
}

MRI_METHOD(input_dir8) {
  scoped_refptr<content::Input> input = MriGetGlobalRunner()->input();
  return rb_fix_new(input->Dir8());
}

void InitInputBinding() {
  VALUE module = rb_define_module("Input");

  MriDefineModuleFunction(module, "update", input_update);
  MriDefineModuleFunction(module, "press?", input_is_pressed);
  MriDefineModuleFunction(module, "trigger?", input_is_triggered);
  MriDefineModuleFunction(module, "repeat?", input_is_repeated);
  MriDefineModuleFunction(module, "dir4", input_dir4);
  MriDefineModuleFunction(module, "dir8", input_dir8);
}

}  // namespace binding
