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
#include "content/public/touch.h"
#include "content/worker/binding_worker.h"
#include "content/worker/content_params.h"
#include "content/worker/worker_share.h"

#include <thread>

namespace content {

class BindingRunner : public base::RefCounted<BindingRunner> {
 public:
  BindingRunner(WorkerShareData* share_data);

  BindingRunner(const BindingRunner&) = delete;
  BindingRunner& operator=(const BindingRunner&) = delete;

  void InitBindingComponents(ContentInitParams& params);
  void BindingMain();

  void RequestQuit();
  void RequestReset();
  void ClearResetFlag();

  bool CheckRunnerFlags();
  void RaiseRunnerFlags();

  scoped_refptr<CoreConfigure> config() const { return share_data_->config; }
  RGSSVersion rgss_version() { return config()->content_version(); }
  WorkerShareData* share_data() { return share_data_; }

  scoped_refptr<Graphics> graphics() const { return graphics_; }
  scoped_refptr<Input> input() const { return input_; }
  scoped_refptr<Audio> audio() const { return audio_; }
  scoped_refptr<Mouse> mouse() const { return mouse_; }
  scoped_refptr<Touch> touch() const { return touch_; }

  base::WeakPtr<BindingRunner> AsWeakPtr() {
    return weak_ptr_factory_.GetWeakPtr();
  }

 private:
  void BindingFuncMain();

  WorkerShareData* share_data_;
  std::unique_ptr<std::thread> runner_thread_;
  std::unique_ptr<BindingEngine> binding_engine_;
  scoped_refptr<RenderRunner> renderer_;

  scoped_refptr<Graphics> graphics_;
  scoped_refptr<Input> input_;
  scoped_refptr<Audio> audio_;
  scoped_refptr<Mouse> mouse_;
  scoped_refptr<Touch> touch_;

  std::atomic_bool quit_atomic_;
  std::atomic_bool reset_atomic_;

  base::WeakPtrFactory<BindingRunner> weak_ptr_factory_{this};
};

}  // namespace content

#endif  //! CONTENT_WORKER_BINDING_WORKER_H_
