// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef CONTENT_WORKER_AUDIO_RUNNER_H_
#define CONTENT_WORKER_AUDIO_RUNNER_H_

#include "base/memory/ref_counted.h"
#include "base/worker/thread_worker.h"
#include "content/config/core_config.h"

#include "soloud.h"

namespace content {

class AudioRunner : public base::SequencedTaskRunner {
 public:
  AudioRunner();
  ~AudioRunner();

  AudioRunner(const AudioRunner&) = delete;
  AudioRunner& operator=(const AudioRunner&) = delete;

  void InitAudioComponents(scoped_refptr<CoreConfigure> config);

  void PostTask(base::OnceClosure task) override;
  void WaitForSync() override;

 private:
  void InitAudioInternal();
  void DestroyAudioInternal();

  std::unique_ptr<base::ThreadWorker> worker_;
  SoLoud::Soloud core_;

  base::WeakPtrFactory<AudioRunner> weak_ptr_factory_{this};
};

}  // namespace content

#endif  //! CONTENT_WORKER_AUDIO_RUNNER_H_
