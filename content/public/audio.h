// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_AUDIO_H_
#define CONTENT_PUBLIC_AUDIO_H_

#include "base/memory/ref_counted.h"
#include "content/worker/audio_runner.h"

namespace content {

class Audio final : public base::RefCounted<Audio> {
 public:
  Audio(scoped_refptr<AudioRunner> runner);
  ~Audio();

  Audio(const Audio&) = delete;
  Audio& operator=(const Audio&) = delete;

  void SetupMidi();
  void BGMPlay(const std::string filename,
               int volume = 100,
               int pitch = 100,
               float pos = 0);
  void BGMStop();
  void BGMFade(int time);
  float BGMPos();

  void BGSPlay(const std::string filename,
               int volume = 100,
               int pitch = 100,
               float pos = 0);
  void BGSStop();
  void BGSFade(int time);
  float BGSPos();

  void MEPlay(const std::string filename, int volume = 100, int pitch = 100);
  void MEStop();
  void MEFade(int time);

  void SEPlay(const std::string filename, int volume = 100, int pitch = 100);
  void SEStop();

 private:
  scoped_refptr<AudioRunner> runner_;
};

}  // namespace content

#endif  // !CONTENT_PUBLIC_AUDIO_H_
