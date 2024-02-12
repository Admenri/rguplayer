// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "content/public/audio.h"

#include "SDL_timer.h"

namespace content {

Audio::Audio(filesystem::Filesystem* file_reader,
             scoped_refptr<AudioRunner> runner)
    : file_reader_(file_reader), runner_(runner) {
  me_watcher_.reset(new std::jthread(&Audio::MEMonitorInternal, runner_));
}

Audio::~Audio() {
  me_watcher_.reset();
}

void Audio::SetupMidi() {
  // TODO: unsupport MIDI
}

void Audio::BGMPlay(const std::string& filename,
                    int volume,
                    int pitch,
                    float pos) {
  file_reader_->OpenRead(
      filename, base::BindRepeating(
                    [](scoped_refptr<AudioRunner> runner,
                       const std::string& filename, int volume, int pitch,
                       float pos, SDL_RWops* ops, const std::string& ext) {
                      runner->PlayStream(AudioRunner::SlotBGM, filename, ops,
                                         volume, pitch, pos, true);
                      if (runner->IsStreamPlaying(AudioRunner::SlotME))
                        runner->PauseStream(AudioRunner::SlotBGM, true);

                      return true;
                    },
                    runner_, filename, volume, pitch, pos));
}

void Audio::BGMStop() {
  runner_->StopStream(AudioRunner::SlotBGM);
}

void Audio::BGMFade(int time) {
  runner_->FadeOutStream(AudioRunner::SlotBGM, time);
}

float Audio::BGMPos() {
  return runner_->StreamPos(AudioRunner::SlotBGM);
}

void Audio::BGSPlay(const std::string& filename,
                    int volume,
                    int pitch,
                    float pos) {
  file_reader_->OpenRead(
      filename, base::BindRepeating(
                    [](scoped_refptr<AudioRunner> runner,
                       const std::string& filename, int volume, int pitch,
                       float pos, SDL_RWops* ops, const std::string& ext) {
                      runner->PlayStream(AudioRunner::SlotBGS, filename, ops,
                                         volume, pitch, pos, true);

                      return true;
                    },
                    runner_, filename, volume, pitch, pos));
}

void Audio::BGSStop() {
  runner_->StopStream(AudioRunner::SlotBGS);
}

void Audio::BGSFade(int time) {
  runner_->FadeOutStream(AudioRunner::SlotBGS, time);
}

float Audio::BGSPos() {
  return runner_->StreamPos(AudioRunner::SlotBGS);
}

void Audio::MEPlay(const std::string& filename, int volume, int pitch) {
  file_reader_->OpenRead(
      filename,
      base::BindRepeating(
          [](scoped_refptr<AudioRunner> runner, const std::string& filename,
             int volume, int pitch, SDL_RWops* ops, const std::string& ext) {
            runner->PlayStream(AudioRunner::SlotME, filename, ops, volume,
                               pitch, 0, false);

            return true;
          },
          runner_, filename, volume, pitch));

  runner_->PauseStream(AudioRunner::SlotBGM, true);
}

void Audio::MEStop() {
  runner_->StopStream(AudioRunner::SlotME);
}

void Audio::MEFade(int time) {
  runner_->FadeOutStream(AudioRunner::SlotME, time);
}

void Audio::SEPlay(const std::string& filename, int volume, int pitch) {
  file_reader_->OpenRead(
      filename,
      base::BindRepeating(
          [](scoped_refptr<AudioRunner> runner, const std::string& filename,
             int volume, int pitch, SDL_RWops* ops, const std::string& ext) {
            runner->SoundEmit(filename, ops, volume, pitch);

            return true;
          },
          runner_, filename, volume, pitch));
}

void Audio::SEStop() {
  runner_->StopAllEmit();
}

void Audio::Reset() {
  runner_->Reset();
}

void Audio::MEMonitorInternal(std::stop_token token,
                              scoped_refptr<AudioRunner> runner) {
  while (!token.stop_requested()) {
    if (runner->IsStreamPaused(AudioRunner::SlotBGM) &&
        !runner->IsStreamPlaying(AudioRunner::SlotME)) {
      runner->FadeInStream(AudioRunner::SlotBGM, 500);
      runner->PauseStream(AudioRunner::SlotBGM, false);
    }

    SDL_Delay(10);
  }
}

}  // namespace content
