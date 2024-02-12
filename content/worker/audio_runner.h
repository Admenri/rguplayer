// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef CONTENT_WORKER_AUDIO_RUNNER_H_
#define CONTENT_WORKER_AUDIO_RUNNER_H_

#include "base/memory/ref_counted.h"
#include "base/worker/thread_worker.h"
#include "content/config/core_config.h"

#include "SDL_rwops.h"

#include "soloud.h"
#include "soloud_wav.h"

#include <array>
#include <map>

namespace content {

class AudioRunner : public base::RefCounted<AudioRunner> {
 public:
  using Slot = enum {
    SlotBGM = 0,
    SlotBGS,
    SlotME,

    SlotNums,
  };

  AudioRunner();
  ~AudioRunner();

  AudioRunner(const AudioRunner&) = delete;
  AudioRunner& operator=(const AudioRunner&) = delete;

  void InitAudioComponents(scoped_refptr<CoreConfigure> config);

  void* ReadMemFile(SDL_RWops* ops, size_t* size, bool freesrc);

  void PlayStream(Slot slot,
                  const std::string& stream_id,
                  SDL_RWops* ops,
                  int volume,
                  int pitch,
                  float pos,
                  bool loop);
  void StopStream(Slot slot);
  void FadeOutStream(Slot slot, int duration);
  float StreamPos(Slot slot);
  void PauseStream(Slot slot, bool paused);
  void FadeInStream(Slot slot, int duration);
  bool IsStreamPlaying(Slot slot);
  bool IsStreamPaused(Slot slot);

  void SoundEmit(const std::string& emit_id,
                 SDL_RWops* ops,
                 int volume,
                 int pitch);
  void StopAllEmit();

  void Reset();

  base::WeakPtr<AudioRunner> AsWeakPtr() {
    return weak_ptr_factory_.GetWeakPtr();
  }

 private:
  void InitAudioInternal();
  void DestroyAudioInternal();

  struct SlotStream {
    std::unique_ptr<SoLoud::Wav> source;
    std::string stream_id;
    SoLoud::handle handle = 0;
    int volume = 100;
    int pitch = 100;
  };

  SoLoud::Soloud core_;

  std::list<std::unique_ptr<SoLoud::Wav>> emit_cache_;
  std::array<SlotStream, SlotNums> slot_stream_;

  base::WeakPtrFactory<AudioRunner> weak_ptr_factory_{this};
};

}  // namespace content

#endif  //! CONTENT_WORKER_AUDIO_RUNNER_H_
