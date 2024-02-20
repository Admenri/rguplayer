// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_AUDIO_H_
#define CONTENT_PUBLIC_AUDIO_H_

#include "base/memory/ref_counted.h"
#include "components/filesystem/filesystem.h"
#include "content/config/core_config.h"

#include "SDL_audio.h"
#include "soloud.h"
#include "soloud_wav.h"

#include <list>

namespace content {

class Audio final : public base::RefCounted<Audio> {
 public:
  Audio(base::WeakPtr<filesystem::Filesystem> file_reader,
        scoped_refptr<CoreConfigure> config);
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

  SDL_AudioDeviceID& output_device() { return output_device_; }
  SDL_AudioStream*& soloud_stream() { return soloud_stream_; }
  SDL_AudioSpec& soloud_spec() { return soloud_spec_; }

 private:
  void InitAudioDeviceInternal();
  void DestroyAudioDeviceInternal();
  void MeMonitorInternal();

  SoLoud::Soloud core_;
  SDL_AudioDeviceID output_device_;
  SDL_AudioStream* soloud_stream_;
  SDL_AudioSpec soloud_spec_;

  struct SlotInfo {
    std::unique_ptr<SoLoud::Wav> source;
    std::string filename;
    SoLoud::handle play_handle = 0;
  };

  SlotInfo bgm_;
  SlotInfo bgs_;
  SlotInfo me_;
  std::list<std::unique_ptr<SoLoud::Wav>> se_cache_;

  base::WeakPtr<filesystem::Filesystem> file_reader_;
  scoped_refptr<CoreConfigure> config_;
  std::unique_ptr<std::jthread> me_watcher_;
  std::atomic_bool quit_flag_;
};

}  // namespace content

#endif  // !CONTENT_PUBLIC_AUDIO_H_
