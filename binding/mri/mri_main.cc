// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "binding/mri/mri_util.h"
#include "ruby/version.h"

#include "binding/mri/mri_main.h"

#include "binding/mri/init_bitmap.h"
#include "binding/mri/init_corefile.h"
#include "binding/mri/init_font.h"
#include "binding/mri/init_table.h"
#include "binding/mri/init_utility.h"
#include "binding/mri/init_viewport.h"
#include "content/worker/binding_worker.h"

#include "binding/rpg/module_rpg1.rb.xxd"
#include "binding/rpg/module_rpg2.rb.xxd"
#include "binding/rpg/module_rpg3.rb.xxd"

#include "zlib.h"

extern "C" {
void rb_call_builtin_inits();
}

namespace binding {

scoped_refptr<content::BindingRunner> g_mri_manager;

namespace {

using EvalParamater = struct {
  VALUE string;
  VALUE filename;
};

// eval script, nil, title
static VALUE EvalInternal(EvalParamater* arg) {
  VALUE argv[] = {arg->string, Qnil, arg->filename};
  return rb_funcall2(Qnil, rb_intern("eval"), 3, argv);
}

static VALUE EvalString(VALUE string, VALUE filename, int* state) {
  EvalParamater arg = {string, filename};
  return rb_protect((VALUE(*)(VALUE))EvalInternal, (VALUE)&arg, state);
}

static void ParseExeceptionInfo(VALUE exc,
                                const BindingEngineMri::BacktraceData& btData) {
  VALUE exeception_name = rb_class_path(rb_obj_class(exc));
  VALUE backtrace = rb_funcall2(exc, rb_intern("backtrace"), 0, NULL);
  VALUE message = rb_funcall2(exc, rb_intern("message"), 0, NULL);
  VALUE backtrace_front = rb_ary_entry(backtrace, 0);

  VALUE ds = rb_sprintf("%" PRIsVALUE ": %" PRIsVALUE " (%" PRIsVALUE ")",
                        backtrace_front, exc, exeception_name);
  LOG(INFO) << "[Binding] " << StringValueCStr(ds);
}

}  // namespace

BindingEngineMri::BindingEngineMri() {}

BindingEngineMri::~BindingEngineMri() {}

void BindingEngineMri::InitializeBinding(
    scoped_refptr<content::BindingRunner> binding_host) {
  binding_ = binding_host;
  g_mri_manager = binding_host;
  scoped_refptr<content::CoreConfigure> config = binding_->config();

  int argc = 0;
  char** argv = 0;
  ruby_sysinit(&argc, &argv);

  RUBY_INIT_STACK;
  ruby_init();

  ruby_init_loadpath();
  rb_call_builtin_inits();

  rb_enc_set_default_internal(rb_enc_from_encoding(rb_utf8_encoding()));
  rb_enc_set_default_external(rb_enc_from_encoding(rb_utf8_encoding()));

  switch (config->version()) {
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

  MriInitException(config->version() == content::CoreConfigure::RGSS3);

  InitCoreFileBinding();
  InitUtilityBinding();
  InitTableBinding();
  InitViewportBinding();
  InitFontBinding();
  InitBitmapBinding();

  LOG(INFO) << "[Binding] CRuby Interpreter Version: " << RUBY_API_VERSION_CODE;
  LOG(INFO) << "[Binding] CRuby Interpreter Platform: " << RUBY_PLATFORM;
}

void BindingEngineMri::RunBindingMain() {
  LOG(INFO) << "[Binding] Load packed scripts and run.";

  LoadPackedScripts();
}

void BindingEngineMri::QuitRequired() {}

void BindingEngineMri::FinalizeBinding() {
  VALUE exc = rb_errinfo();
  if (!NIL_P(exc) && !rb_obj_is_kind_of(exc, rb_eSystemExit))
    ParseExeceptionInfo(exc, backtrace_);

  ruby_cleanup(0);

  g_mri_manager.reset();

  LOG(INFO) << "[Binding] Quit mri binding engine.";
}

void BindingEngineMri::LoadPackedScripts() {
  scoped_refptr<content::CoreConfigure> config = binding_->config();

  VALUE packed_scripts;

  try {
    packed_scripts = MriLoadData(config->game_scripts(), false);
  } catch (const base::Exception& exception) {
    LOG(INFO) << exception.GetErrorMessage();
    return;
  }

  if (!RB_TYPE_P(packed_scripts, RUBY_T_ARRAY)) {
    LOG(INFO) << "Failed to read script data.";
    return;
  }

  rb_gv_set("$RGSS_SCRIPTS", packed_scripts);

  long scripts_count = RARRAY_LEN(packed_scripts);
  std::string zlib_decode_buffer(0x1000, 0);

  for (long i = 0; i < scripts_count; ++i) {
    VALUE script = rb_ary_entry(packed_scripts, i);

    // 0 -> ScriptID for Editor
    VALUE script_name = rb_ary_entry(script, 1);
    VALUE script_src = rb_ary_entry(script, 2);
    unsigned long buffer_size;

    int zlib_result = Z_OK;

    while (true) {
      uint8_t* buffer_ptr =
          reinterpret_cast<uint8_t*>(zlib_decode_buffer.data());
      buffer_size = zlib_decode_buffer.length();

      const uint8_t* source_ptr =
          reinterpret_cast<const uint8_t*>(RSTRING_PTR(script_src));
      const uint32_t source_size = RSTRING_LEN(script_src);

      zlib_result =
          uncompress(buffer_ptr, &buffer_size, source_ptr, source_size);

      buffer_ptr[buffer_size] = '\0';

      if (zlib_result != Z_BUF_ERROR)
        break;

      zlib_decode_buffer.resize(zlib_decode_buffer.size() * 2);
    }

    if (zlib_result != Z_OK) {
      static char buffer[256];
      snprintf(buffer, sizeof(buffer), "Error decoding script %ld: '%s'", i,
               RSTRING_PTR(script_name));

      LOG(INFO) << buffer;
      break;
    }

    rb_ary_store(script, 3, rb_str_new_cstr(zlib_decode_buffer.c_str()));
  }

  VALUE exc = rb_gv_get("$!");
  if (exc != Qnil)
    return;

  while (true) {
    for (long i = 0; i < scripts_count; ++i) {
      VALUE script = rb_ary_entry(packed_scripts, i);
      const char* script_name = RSTRING_PTR(rb_ary_entry(script, 1));
      VALUE script_src = rb_ary_entry(script, 3);

      VALUE utf8_string =
          MriStringUTF8(RSTRING_PTR(script_src), RSTRING_LEN(script_src));

      char filename_buffer[512] = {0};
      int len = snprintf(filename_buffer, sizeof(filename_buffer), "%03ld: %s",
                         i, script_name);

      VALUE filename = MriStringUTF8(filename_buffer, len);
      backtrace_.insert(std::make_pair(filename_buffer, script_name));

      int state;
      EvalString(utf8_string, filename, &state);
      if (state)
        break;
    }

    VALUE exc = rb_gv_get("$!");
    if (rb_obj_class(exc) != MriGetException(RGSSReset))
      break;

    LOG(INFO) << "Reset";
  }
}

}  // namespace binding
