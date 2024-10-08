// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef CONTENT_WORKER_WORKER_SCHEDULER_H_
#define CONTENT_WORKER_WORKER_SCHEDULER_H_

namespace content {

class WorkerScheduler {
 public:
  WorkerScheduler();
  ~WorkerScheduler();

  WorkerScheduler(const WorkerScheduler&) = delete;
  WorkerScheduler& operator=(const WorkerScheduler&) = delete;

 private:
};

}  // namespace content

#endif  //! CONTENT_WORKER_WORKER_SCHEDULER_H_
