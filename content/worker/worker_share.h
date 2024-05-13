// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef CONTENT_WORKER_WORKER_SHARE_H_
#define CONTENT_WORKER_WORKER_SHARE_H_

#include "base/third_party/concurrentqueue/concurrentqueue.h"
#include "components/filesystem/filesystem.h"
#include "content/config/core_config.h"
#include "ui/widget/widget.h"

#include "SDL_events.h"

#include <atomic>
#include <queue>

namespace content {

/* Storage structure of the engine shared data.
 * Used in multi worker build on different threads.
 * Created and managed in worker host tree manager. */
struct WorkerShareData {
  // Engine configure storage
  scoped_refptr<CoreConfigure> config;

  // Generic IO filesystem
  std::unique_ptr<filesystem::Filesystem> filesystem;

  // Custom allocate event user id
  uint32_t user_event_id;

  // Renderer data storage
  base::WeakPtr<ui::Widget> window;

  // Event task runner
  scoped_refptr<base::SequencedTaskRunner> event_runner;

  // Sync point
  struct {
    std::atomic_bool require;
    std::atomic_bool signal;
  } background_sync;

  // GUI Event Queue
  bool disable_gui_key_input = false;
  moodycamel::ConcurrentQueue<SDL_Event> event_queue;
  base::RepeatingClosure create_button_settings_gui;
};

}  // namespace content

#endif  //! CONTENT_WORKER_WORKER_SHARE_H_
