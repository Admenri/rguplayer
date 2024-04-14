// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef CONTENT_WORKER_WORKER_SHARE_H_
#define CONTENT_WORKER_WORKER_SHARE_H_

#include "components/filesystem/filesystem.h"
#include "content/config/core_config.h"
#include "ui/widget/widget.h"

#include <atomic>

namespace content {

/* Storage structure of the engine shared data.
 * Used in multi worker build on different threads.
 * Created and managed in worker host tree manager. */
struct WorkerShareData {
  // Base path of engine
  std::string argv0;

  // Engine configure storage
  scoped_refptr<CoreConfigure> config;

  // Generic IO filesystem
  std::unique_ptr<filesystem::Filesystem> filesystem;

  // Custom allocate event user id
  uint32_t user_event_id;

  // Renderer data storage
  base::WeakPtr<ui::Widget> window;
};

}  // namespace content

#endif  //! CONTENT_WORKER_WORKER_SHARE_H_
