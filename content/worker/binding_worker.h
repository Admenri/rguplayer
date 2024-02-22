// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef CONTENT_WORKER_BINDING_WORKER_H_
#define CONTENT_WORKER_BINDING_WORKER_H_

#include "base/memory/ref_counted.h"
#include "components/filesystem/filesystem.h"
#include "content/public/audio.h"
#include "content/public/graphics.h"
#include "content/public/input.h"
#include "content/public/mouse.h"
#include "content/worker/content_params.h"

#include <thread>

namespace content {

class BindingRunner : public base::RefCounted<BindingRunner> {
 public:
  BindingRunner() = default;

  BindingRunner(const BindingRunner&) = delete;
  BindingRunner& operator=(const BindingRunner&) = delete;

  void InitBindingComponents(ContentInitParams& params);
  void BindingMain(uint32_t event_id);
  void RequestQuit();
  void RequestReset();
  void ClearResetFlag();

  bool CheckFlags();
  void RaiseFlags();

  RGSSVersion rgss_version() { return config_->content_version(); }
  scoped_refptr<CoreConfigure> config() const { return config_; }
  uint32_t user_event_id() { return user_event_id_; }
  filesystem::Filesystem* filesystem() { return file_manager_.get(); }

  scoped_refptr<Graphics> graphics() const { return graphics_; }
  scoped_refptr<Input> input() const { return input_; }
  scoped_refptr<Audio> audio() const { return audio_; }
  scoped_refptr<Mouse> mouse() const { return mouse_; }

  base::WeakPtr<BindingRunner> AsWeakPtr() {
    return weak_ptr_factory_.GetWeakPtr();
  }

 private:
  void BindingFuncMain();

  scoped_refptr<CoreConfigure> config_;
  std::unique_ptr<std::thread> runner_thread_;

  scoped_refptr<RenderRunner> renderer_;
  scoped_refptr<Graphics> graphics_;
  scoped_refptr<Input> input_;
  scoped_refptr<Audio> audio_;
  scoped_refptr<Mouse> mouse_;

  base::Vec2i initial_resolution_;
  base::WeakPtr<ui::Widget> window_;
  base::AtomicFlag quit_atomic_;
  base::AtomicFlag reset_atomic_;
  uint32_t user_event_id_;

  std::string argv0_;
  std::unique_ptr<filesystem::Filesystem> file_manager_;
  std::unique_ptr<BindingEngine> binding_engine_;

  base::WeakPtrFactory<BindingRunner> weak_ptr_factory_{this};
};

}  // namespace content

#endif  //! CONTENT_WORKER_BINDING_WORKER_H_
