// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "binding/mri/init_input.h"

#include "content/public/input.h"

#include <array>

namespace binding {

namespace {

std::array<std::string, 20> kKeyboardBindings = {
    "DOWN",  "LEFT", "RIGHT", "UP",

    "F5",    "F6",   "F7",    "F8", "F9",

    "SHIFT", "CTRL", "ALT",

    "A",     "B",    "C",     "X",  "Y",  "Z", "L", "R",
};

}  // namespace

MRI_METHOD(input_update) {
  scoped_refptr<content::Input> input = MriGetGlobalRunner()->input();

  input->Update();

  return Qnil;
}

MRI_METHOD(input_is_pressed) {
  scoped_refptr<content::Input> input = MriGetGlobalRunner()->input();

  std::string key;
  MriParseArgsTo(argc, argv, "n", &key);

  bool v = input->IsPressed(key);
  return MRI_BOOL_NEW(v);
}

MRI_METHOD(input_is_triggered) {
  scoped_refptr<content::Input> input = MriGetGlobalRunner()->input();

  std::string key;
  MriParseArgsTo(argc, argv, "n", &key);

  bool v = input->IsTriggered(key);
  return MRI_BOOL_NEW(v);
}

MRI_METHOD(input_is_repeated) {
  scoped_refptr<content::Input> input = MriGetGlobalRunner()->input();

  std::string key;
  MriParseArgsTo(argc, argv, "n", &key);

  bool v = input->IsRepeated(key);
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

  for (auto& it : kKeyboardBindings)
    rb_const_set(module, rb_intern(it.c_str()), rb_str_new2(it.c_str()));
}

}  // namespace binding
