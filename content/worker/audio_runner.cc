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

AudioRunner::AudioRunner() {}

AudioRunner::~AudioRunner() {
  DestroyAudioInternal();
}

void AudioRunner::InitAudioComponents(scoped_refptr<CoreConfigure> config) {
  InitAudioInternal();
}

void* AudioRunner::ReadMemFile(SDL_RWops* src, size_t* datasize, bool freesrc) {
  const int FILE_CHUNK_SIZE = 1024;
  Sint64 size, size_total;
  size_t size_read;
  char *data = NULL, *newdata;
  SDL_bool loading_chunks = SDL_FALSE;

  if (!src) {
    SDL_InvalidParamError("src");
    return NULL;
  }

  size = SDL_RWsize(src);
  if (size < 0) {
    size = FILE_CHUNK_SIZE;
    loading_chunks = SDL_TRUE;
  }
  if (size >= SDL_SIZE_MAX) {
    goto done;
  }
  data = new char[(size_t)(size + 1)];
  if (!data) {
    goto done;
  }

  size_total = 0;
  for (;;) {
    if (loading_chunks) {
      if ((size_total + FILE_CHUNK_SIZE) > size) {
        size = (size_total + FILE_CHUNK_SIZE);
        if (size >= SDL_SIZE_MAX) {
          newdata = NULL;
        } else {
          delete[] data;
          newdata = new char[(size_t)(size + 1)];
        }
        if (!newdata) {
          delete[] data;
          data = NULL;
          goto done;
        }
        data = newdata;
      }
    }

    size_read = SDL_RWread(src, data + size_total, (size_t)(size - size_total));
    if (size_read > 0) {
      size_total += size_read;
      continue;
    }

    /* The stream status will remain set for the caller to check */
    break;
  }

  if (datasize) {
    *datasize = (size_t)size_total;
  }
  data[size_total] = '\0';

done:
  if (freesrc && src) {
    SDL_RWclose(src);
  }
  return data;
}

void AudioRunner::PlayStream(Slot slot,
                             const std::string& stream_id,
                             SDL_RWops* ops,
                             int volume,
                             int pitch,
                             float pos,
                             bool loop) {
  SlotStream& stream = slot_stream_[slot];

  if (stream.stream_id != stream_id ||
      !core_.isValidVoiceHandle(stream.handle)) {
    stream.stream_id = stream_id;
    stream.source.reset(new SoLoud::Wav);
    size_t size = 0;
    void* mem = ReadMemFile(ops, &size, SDL_TRUE);
    stream.source->loadMem((uint8_t*)mem, size);
    stream.handle = core_.play(*stream.source);
  }

  stream.volume = volume;
  stream.pitch = pitch;

  core_.setLooping(stream.handle, loop);
  core_.setVolume(stream.handle, volume / 100.0f);
  core_.setRelativePlaySpeed(stream.handle, pitch / 100.0f);
  if (pos)
    core_.seek(stream.handle, pos / 1000.0f);
}

void AudioRunner::StopStream(Slot slot) {
  SlotStream& stream = slot_stream_[slot];
  stream.stream_id.clear();
  core_.stop(stream.handle);
  stream.source.reset();
}

void AudioRunner::FadeOutStream(Slot slot, int duration) {
  SlotStream& stream = slot_stream_[slot];
  if (core_.isValidVoiceHandle(stream.handle))
    core_.fadeVolume(stream.handle, 0, duration * 0.001);
}

float AudioRunner::StreamPos(Slot slot) {
  SlotStream& stream = slot_stream_[slot];

  if (core_.isValidVoiceHandle(stream.handle))
    return core_.getStreamPosition(stream.handle) * 1000;

  return 0.0f;
}

void AudioRunner::PauseStream(Slot slot, bool paused) {
  SlotStream& stream = slot_stream_[slot];
  if (core_.isValidVoiceHandle(stream.handle))
    core_.setPause(stream.handle, paused);
}

void AudioRunner::FadeInStream(Slot slot, int duration) {
  SlotStream& stream = slot_stream_[slot];
  if (core_.isValidVoiceHandle(stream.handle)) {
    core_.setVolume(stream.handle, 0);
    core_.fadeVolume(stream.handle, stream.volume / 100.0f, duration * 0.001);
  }
}

bool AudioRunner::IsStreamPlaying(Slot slot) {
  SlotStream& stream = slot_stream_[slot];
  return core_.isValidVoiceHandle(stream.handle);
}

bool AudioRunner::IsStreamPaused(Slot slot) {
  SlotStream& stream = slot_stream_[slot];
  if (core_.isValidVoiceHandle(stream.handle))
    return core_.getPause(stream.handle);
  return false;
}

void AudioRunner::SoundEmit(const std::string& emit_id,
                            SDL_RWops* ops,
                            int volume,
                            int pitch) {
  // Clean stoped se
  auto dit = std::remove_if(emit_cache_.begin(), emit_cache_.end(),
                            [&](const std::unique_ptr<SoLoud::Wav>& it) {
                              return !core_.countAudioSource(*it);
                            });
  emit_cache_.erase(dit, emit_cache_.end());

  // Load new source
  size_t size = 0;
  void* mem = ReadMemFile(ops, &size, SDL_TRUE);
  std::unique_ptr<SoLoud::Wav> source(new SoLoud::Wav);
  source->loadMem((uint8_t*)mem, size);
  auto handle = core_.play(*source);
  core_.setVolume(handle, volume / 100.0f);
  core_.setRelativePlaySpeed(handle, pitch / 100.0f);
  emit_cache_.push_back(std::move(source));
}

void AudioRunner::StopAllEmit() {
  for (auto& it : emit_cache_)
    core_.stopAudioSource(*it);
}

void AudioRunner::Reset() {
  core_.stopAll();
  emit_cache_.clear();
}

void AudioRunner::InitAudioInternal() {
  if (core_.init(sdlbackend_init, 0, 0) != SoLoud::SO_NO_ERROR)
    LOG(INFO) << "[Content] Failed to initialize audio core.";
}

void AudioRunner::DestroyAudioInternal() {
  if (!g_audio_device)
    return;

  LOG(INFO) << "[Content] Finalize audio core.";

  emit_cache_.clear();
  core_.deinit();
}

}  // namespace content
