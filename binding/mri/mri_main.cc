// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "ruby.h"
#include "ruby/encoding.h"

#include "binding/mri/mri_main.h"

#include "binding/rpg/module_rpg1.rb.xxd"
#include "binding/rpg/module_rpg2.rb.xxd"
#include "binding/rpg/module_rpg3.rb.xxd"

#include "content/worker/binding_worker.h"

extern "C" {
void rb_call_builtin_inits();
}

namespace binding {

void InitUtilityBinding();

BindingEngineMri::BindingEngineMri() {}

BindingEngineMri::~BindingEngineMri() {}

void BindingEngineMri::InitializeBinding(
    scoped_refptr<content::BindingRunner> binding_host) {
  binding_ = binding_host;

  int argc = 0;
  char** argv = 0;
  ruby_sysinit(&argc, &argv);

  RUBY_INIT_STACK;
  ruby_init();

  ruby_init_loadpath();
  rb_call_builtin_inits();

  rb_enc_set_default_internal(rb_enc_from_encoding(rb_utf8_encoding()));
  rb_enc_set_default_external(rb_enc_from_encoding(rb_utf8_encoding()));

  switch (binding_->config()->version()) {
    case content::CoreConfigure::RGSS1:
      rb_eval_string(module_rpg1);
      break;
    case content::CoreConfigure::RGSS2:
      rb_eval_string(module_rpg2);
      break;
    case content::CoreConfigure::RGSS3:
      rb_eval_string(module_rpg3);
      break;
    default:
      break;
  }

  InitUtilityBinding();

  LOG(INFO) << "Mri binding initialized.";
}

void BindingEngineMri::RunBindingMain() {
  LOG(INFO) << "Execute Mri binding main loop.";
  while (!binding_->quit_required()) {
    binding_->graphics()->Update();
  }
}

void BindingEngineMri::QuitRequired() {}

void BindingEngineMri::FinalizeBinding() {
  ruby_cleanup(0);

  LOG(INFO) << "Quit Mri binding.";
}

}  // namespace binding
