// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "content/worker/audio_runner.h"

#include "SDL.h"
#include "SDL_audio.h"

#include <stdlib.h>

namespace content {

namespace {

void soloud_sdl_audiomixer(void* userdata,
                           SDL_AudioStream* stream,
                           int additional_amount,
                           int total_amount) {}

void soloud_sdl_deinit(SoLoud::Soloud* aSoloud) {}

SoLoud::result sdlbackend_init(SoLoud::Soloud* aSoloud,
                               unsigned int aFlags,
                               unsigned int aSamplerate,
                               unsigned int aBuffer,
                               unsigned int aChannels) {
  return 0;
}

}  // namespace

AudioRunner::AudioRunner() : worker_(std::make_unique<base::ThreadWorker>()) {}

AudioRunner::~AudioRunner() {}

void AudioRunner::InitAudioComponents(scoped_refptr<CoreConfigure> config) {
  LOG(INFO) << "[Audio] Current driver: " << SDL_GetCurrentAudioDriver();

  int device_num = 0;
  SDL_AudioDeviceID* devices = SDL_GetAudioOutputDevices(&device_num);
  for (int i = 0; i < device_num; ++i) {
    LOG(INFO) << "[Audio] Device" << i + 1 << ": "
              << SDL_GetAudioDeviceName(devices[i]);
  }

  worker_->Start(base::RunLoop::MessagePumpType::Worker);
  worker_->WaitUntilStart();
}

void AudioRunner::PostTask(base::OnceClosure task) {
  worker_->task_runner()->PostTask(std::move(task));
}

void AudioRunner::WaitForSync() {
  worker_->task_runner()->WaitForSync();
}

void AudioRunner::InitAudioInternal() {
  core_.init(sdlbackend_init, 0, 0);
}

void AudioRunner::DestroyAudioInternal() {
  core_.deinit();
}

}  // namespace content
