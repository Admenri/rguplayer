// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "content/public/audio.h"

#include "base/exceptions/exception.h"
#include "content/common/command_ids.h"
#include "third_party/imgui/imgui.h"

#include "SDL_timer.h"

namespace content {

Audio::Audio(WorkerShareData* share_data) : share_data_(share_data) {
  if (share_data_->config->disable_audio()) {
    LOG(INFO) << "[Content] Disable Audio module.";
    return;
  }

  audio_runner_ = std::make_unique<base::ThreadWorker>();
  audio_runner_->Start(base::RunLoop::MessagePumpType::Worker);
  audio_runner_->WaitUntilStart();

  // Bind settings function
  share_data_->create_audio_settings_gui = base::BindRepeating(
      &Audio::DrawAudioSettingsGUI, weak_ptr_factory_.GetWeakPtr());
}

Audio::~Audio() {}

void Audio::SetupMidi() {
  // TODO: unsupport MIDI
  LOG(WARNING) << "[Content] Unsupport MIDI device setup.";
}

void Audio::BGMPlay(const std::string& filename,
                    int volume,
                    int pitch,
                    double pos) {}

void Audio::BGMStop() {}

void Audio::BGMFade(int time) {}

double Audio::BGMPos() {
  return 0.0;
}

void Audio::BGSPlay(const std::string& filename,
                    int volume,
                    int pitch,
                    double pos) {}

void Audio::BGSStop() {}

void Audio::BGSFade(int time) {}

double Audio::BGSPos() {
  return 0.0;
}

void Audio::MEPlay(const std::string& filename, int volume, int pitch) {}

void Audio::MEStop() {}

void Audio::MEFade(int time) {}

void Audio::SEPlay(const std::string& filename, int volume, int pitch) {}

void Audio::SEStop() {}

void Audio::Reset() {}

void Audio::DrawAudioSettingsGUI() {
  scoped_refptr<CoreConfigure> config = share_data_->config;
  if (ImGui::CollapsingHeader(
          config->GetI18NString(IDS_SETTINGS_AUDIO, "Audio").c_str())) {
    float volume = 0.0f;
    ImGui::SliderFloat(
        config->GetI18NString(IDS_AUDIO_VOLUME, "Volume").c_str(), &volume, 0,
        1);
  }
}

}  // namespace content
