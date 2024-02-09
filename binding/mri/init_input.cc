// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "binding/mri/init_input.h"

#include "content/public/input.h"

#include <map>

namespace binding {

namespace {

VALUE g_input_symbol_hash;

const std::map<std::string, int> kKeyboardBindings = {
    {"DOWN", 2},   {"LEFT", 4},  {"RIGHT", 6}, {"UP", 8},

    {"A", 11},     {"B", 12},    {"C", 13},    {"X", 14},  {"Y", 15},
    {"Z", 16},     {"L", 17},    {"R", 18},

    {"SHIFT", 21}, {"CTRL", 22}, {"ALT", 23},

    {"F5", 25},    {"F6", 26},   {"F7", 27},   {"F8", 28}, {"F9", 29},
};

std::string GetButtonSymbol(int argc, VALUE* argv) {
  std::string sym;

  if (FIXNUM_P(*argv)) {
    VALUE str_key = rb_hash_lookup2(g_input_symbol_hash, FIX2INT(*argv),
                                    rb_str_new2(sym.c_str()));
    sym = std::string(RSTRING_PTR(str_key), RSTRING_LEN(str_key));
  } else {
    MriParseArgsTo(argc, argv, "n", &sym);
  }

  return sym;
}

}  // namespace

MRI_METHOD(input_update) {
  scoped_refptr<content::Input> input = MriGetGlobalRunner()->input();

  input->Update();

  return Qnil;
}

MRI_METHOD(input_is_pressed) {
  scoped_refptr<content::Input> input = MriGetGlobalRunner()->input();
  std::string key = GetButtonSymbol(argc, argv);
  bool v = input->IsPressed(key);
  return MRI_BOOL_NEW(v);
}

MRI_METHOD(input_is_triggered) {
  scoped_refptr<content::Input> input = MriGetGlobalRunner()->input();
  std::string key = GetButtonSymbol(argc, argv);
  bool v = input->IsTriggered(key);
  return MRI_BOOL_NEW(v);
}

MRI_METHOD(input_is_repeated) {
  scoped_refptr<content::Input> input = MriGetGlobalRunner()->input();
  std::string key = GetButtonSymbol(argc, argv);
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

  g_input_symbol_hash = rb_hash_new();
  for (auto& it : kKeyboardBindings) {
    ID key = rb_intern(it.first.c_str());

    rb_const_set(module, key, rb_str_new2(it.first.c_str()));

    rb_hash_aset(g_input_symbol_hash, INT2FIX(it.second),
                 rb_str_new(it.first.c_str(), it.first.size()));
  }

  rb_iv_set(module, "_symbol_table", g_input_symbol_hash);
}

}  // namespace binding
