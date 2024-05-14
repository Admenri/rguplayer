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

#include "soloud.h"
#include "soloud_wav.h"

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

  void SetGlobalVolume(int volume = 100);

  SDL_AudioDeviceID& output_device() { return output_device_; }
  SDL_AudioStream*& soloud_stream() { return soloud_stream_; }
  SDL_AudioSpec& soloud_spec() { return soloud_spec_; }

 private:
  friend class Stream;

  struct SlotInfo {
    std::unique_ptr<SoLoud::Wav> source;
    std::string filename;
    SoLoud::handle play_handle = 0;
  };

  void InitAudioDeviceInternal();
  void DestroyAudioDeviceInternal();
  void MeMonitorInternal();

  void PlaySlotInternal(SlotInfo* slot,
                        const std::string& filename,
                        int volume = 100,
                        int pitch = 100,
                        double pos = 0,
                        bool loop = true);
  void StopSlotInternal(SlotInfo* slot);
  void FadeSlotInternal(SlotInfo* slot, int time);
  void GetSlotPosInternal(SlotInfo* slot, double* out);

  void EmitSoundInternal(const std::string& filename,
                         int volume = 100,
                         int pitch = 100);
  void StopEmitInternal();

  void ResetInternal();

  void DrawAudioSettingsGUI();

  std::unique_ptr<base::ThreadWorker> audio_runner_;

  SoLoud::Soloud core_;
  SDL_AudioDeviceID output_device_;
  SDL_AudioStream* soloud_stream_{0};
  SDL_AudioSpec soloud_spec_{0};

  SlotInfo bgm_;
  SlotInfo bgs_;
  SlotInfo me_;
  std::unordered_map<std::string, std::unique_ptr<SoLoud::Wav>> se_cache_;

  WorkerShareData* share_data_;
  std::unique_ptr<std::thread> me_watcher_;
  std::atomic_bool quit_flag_;

  base::WeakPtrFactory<Audio> weak_ptr_factory_{this};
};

}  // namespace content

#endif  // !CONTENT_PUBLIC_AUDIO_H_
