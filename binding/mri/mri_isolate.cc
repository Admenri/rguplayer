// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "binding/mri/mri_isolate.h"

#include "ruby.h"
#include "ruby/encoding.h"

#include "binding/rpg/module_rpg1.rb.xxd"
#include "binding/rpg/module_rpg2.rb.xxd"
#include "binding/rpg/module_rpg3.rb.xxd"

extern "C" {
void rb_call_builtin_inits();
}

void init_mri_binding(int rgss_version) {
  int argc = 0;
  char** argv = 0;
  ruby_sysinit(&argc, &argv);

  RUBY_INIT_STACK;
  ruby_init();

  ruby_init_loadpath();
  rb_call_builtin_inits();

  rb_enc_set_default_internal(rb_enc_from_encoding(rb_utf8_encoding()));
  rb_enc_set_default_external(rb_enc_from_encoding(rb_utf8_encoding()));

  switch (rgss_version) {
    case 0:
      rb_eval_string(module_rpg1);
      break;
    case 1:
      rb_eval_string(module_rpg2);
      break;
    case 2:
      rb_eval_string(module_rpg3);
      break;
    default:
      break;
  }
}

void uninit_mri_binding() {
  ruby_cleanup(0);
}
