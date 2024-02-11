// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "content/public/audio.h"

namespace content {

Audio::Audio(scoped_refptr<AudioRunner> runner) : runner_(runner) {}

Audio::~Audio() {}

void Audio::SetupMidi() {}

void Audio::BGMPlay(const std::string filename,
                    int volume,
                    int pitch,
                    float pos) {}

void Audio::BGMStop() {}

void Audio::BGMFade(int time) {}

float Audio::BGMPos() {
  return 0.0f;
}

void Audio::BGSPlay(const std::string filename,
                    int volume,
                    int pitch,
                    float pos) {}

void Audio::BGSStop() {}

void Audio::BGSFade(int time) {}

float Audio::BGSPos() {
  return 0.0f;
}

void Audio::MEPlay(const std::string filename, int volume, int pitch) {}

void Audio::MEStop() {}

void Audio::MEFade(int time) {}

void Audio::SEPlay(const std::string filename, int volume, int pitch) {}

void Audio::SEStop() {}

}  // namespace content
