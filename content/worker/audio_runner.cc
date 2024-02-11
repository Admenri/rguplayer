// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "content/worker/audio_runner.h"

#include "SDL.h"
#include "SDL_audio.h"

#include "soloud_wav.h"

#include <stdlib.h>

namespace content {

namespace {

SDL_AudioDeviceID g_audio_device;
SDL_AudioStream* g_soloud_stream = nullptr;
SDL_AudioSpec g_soloud_spec = {0};

void soloud_sdl_audiomixer(void* userdata,
                           SDL_AudioStream* stream,
                           int additional_amount,
                           int total_amount) {
  Uint8* data = static_cast<Uint8*>(SDL_malloc(additional_amount));

  int len = additional_amount;
  short* buf = (short*)data;
  SoLoud::Soloud* soloud = (SoLoud::Soloud*)userdata;
  int samples = len / (g_soloud_spec.channels * sizeof(float));
  soloud->mix((float*)buf, samples);

  SDL_PutAudioStreamData(stream, data, additional_amount);
  SDL_free(data);
}

void soloud_sdl_deinit(SoLoud::Soloud* aSoloud) {
  SDL_DestroyAudioStream(g_soloud_stream);
  SDL_CloseAudioDevice(g_audio_device);
}

SoLoud::result sdlbackend_init(SoLoud::Soloud* aSoloud,
                               unsigned int aFlags,
                               unsigned int aSamplerate,
                               unsigned int aBuffer,
                               unsigned int aChannels) {
  g_soloud_spec.freq = aSamplerate;
  g_soloud_spec.format = SDL_AUDIO_F32;
  g_soloud_spec.channels = aChannels;

  g_audio_device =
      SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_OUTPUT, &g_soloud_spec);
  if (!g_audio_device)
    return SoLoud::UNKNOWN_ERROR;

  g_soloud_stream = SDL_CreateAudioStream(&g_soloud_spec, &g_soloud_spec);
  if (!g_soloud_stream)
    return SoLoud::UNKNOWN_ERROR;
  SDL_SetAudioStreamGetCallback(g_soloud_stream, soloud_sdl_audiomixer,
                                aSoloud);

  aSoloud->postinit_internal(g_soloud_spec.freq, aBuffer, aFlags,
                             g_soloud_spec.channels);
  aSoloud->mBackendCleanupFunc = soloud_sdl_deinit;

  SDL_BindAudioStream(g_audio_device, g_soloud_stream);
  SDL_ResumeAudioDevice(g_audio_device);

  return 0;
}

}  // namespace

AudioRunner::AudioRunner() : worker_(std::make_unique<base::ThreadWorker>()) {}

AudioRunner::~AudioRunner() {
  if (worker_) {
    PostTask(base::BindOnce(&AudioRunner::DestroyAudioInternal,
                            weak_ptr_factory_.GetWeakPtr()));
    WaitForSync();
  }
}

void AudioRunner::InitAudioComponents(scoped_refptr<CoreConfigure> config) {
  worker_->Start(base::RunLoop::MessagePumpType::Worker);
  worker_->WaitUntilStart();

  PostTask(base::BindOnce(&AudioRunner::InitAudioInternal,
                          weak_ptr_factory_.GetWeakPtr()));
  worker_->WaitUntilStart();
}

void AudioRunner::PostTask(base::OnceClosure task) {
  if (worker_)
    worker_->task_runner()->PostTask(std::move(task));
}

void AudioRunner::WaitForSync() {
  if (worker_)
    worker_->task_runner()->WaitForSync();
}

void AudioRunner::InitAudioInternal() {
  if (core_.init(sdlbackend_init, 0, 0) != SoLoud::SO_NO_ERROR)
    LOG(INFO) << "[Content] Failed to initialize audio core.";
}

void AudioRunner::DestroyAudioInternal() {
  if (!g_audio_device)
    return;

  LOG(INFO) << "[Content] Finalize audio core.";
  core_.deinit();
}

}  // namespace content
