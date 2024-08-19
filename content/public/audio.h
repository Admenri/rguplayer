// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_AUDIO_H_
#define CONTENT_PUBLIC_AUDIO_H_

#include "base/memory/ref_counted.h"
#include "base/worker/thread_worker.h"
#include "components/filesystem/filesystem.h"
#include "content/config/core_config.h"
#include "content/worker/worker_share.h"

#include "SDL_audio.h"

#include <list>
#include <unordered_map>

namespace content {

class Audio final : public base::RefCounted<Audio> {
 public:
  Audio(WorkerShareData* share_data);
  ~Audio();

  Audio(const Audio&) = delete;
  Audio& operator=(const Audio&) = delete;

  void SetupMidi();
  void BGMPlay(const std::string& filename,
               int volume = 100,
               int pitch = 100,
               double pos = 0);
  void BGMStop();
  void BGMFade(int time);
  double BGMPos();

  void BGSPlay(const std::string& filename,
               int volume = 100,
               int pitch = 100,
               double pos = 0);
  void BGSStop();
  void BGSFade(int time);
  double BGSPos();

  void MEPlay(const std::string& filename, int volume = 100, int pitch = 100);
  void MEStop();
  void MEFade(int time);

  void SEPlay(const std::string& filename, int volume = 100, int pitch = 100);
  void SEStop();

  void Reset();

 private:
  void DrawAudioSettingsGUI();

  WorkerShareData* share_data_;
  std::unique_ptr<base::ThreadWorker> audio_runner_;

  base::WeakPtrFactory<Audio> weak_ptr_factory_{this};
};

}  // namespace content

#endif  // !CONTENT_PUBLIC_AUDIO_H_
