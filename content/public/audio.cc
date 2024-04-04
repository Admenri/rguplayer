// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "content/public/audio.h"

#include "SDL_timer.h"

namespace content {

namespace {

void* read_mem_file(SDL_IOStream* src, size_t* datasize, bool freesrc) {
  const int FILE_CHUNK_SIZE = 1024;
  Sint64 size, size_total;
  size_t size_read;
  char *data = NULL, *newdata;
  SDL_bool loading_chunks = SDL_FALSE;

  if (!src) {
    SDL_InvalidParamError("src");
    return NULL;
  }

  size = SDL_GetIOSize(src);
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

    size_read = SDL_ReadIO(src, data + size_total, (size_t)(size - size_total));
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
    SDL_CloseIO(src);
  }
  return data;
}

void soloud_sdl_audiomixer(void* userdata,
                           SDL_AudioStream* stream,
                           int additional_amount,
                           int total_amount) {
  Uint8* data = static_cast<Uint8*>(SDL_malloc(additional_amount));
  SoLoud::Soloud* soloud = (SoLoud::Soloud*)userdata;
  Audio* manager = static_cast<Audio*>(soloud->mUserData);

  int len = additional_amount;
  short* buf = (short*)data;
  int samples = len / (manager->soloud_spec().channels * sizeof(float));
  soloud->mix((float*)buf, samples);

  SDL_PutAudioStreamData(stream, data, additional_amount);
  SDL_free(data);
}

void soloud_sdl_deinit(SoLoud::Soloud* aSoloud) {
  Audio* manager = static_cast<Audio*>(aSoloud->mUserData);
  SDL_DestroyAudioStream(manager->soloud_stream());
}

SoLoud::result soloud_sdlbackend_init(SoLoud::Soloud* aSoloud,
                                      unsigned int aFlags,
                                      unsigned int aSamplerate,
                                      unsigned int aBuffer,
                                      unsigned int aChannels) {
  Audio* manager = static_cast<Audio*>(aSoloud->mUserData);
  manager->soloud_stream() =
      SDL_CreateAudioStream(&manager->soloud_spec(), &manager->soloud_spec());
  if (!manager->soloud_stream())
    return SoLoud::UNKNOWN_ERROR;
  SDL_SetAudioStreamGetCallback(manager->soloud_stream(), soloud_sdl_audiomixer,
                                aSoloud);

  aSoloud->postinit_internal(manager->soloud_spec().freq, aBuffer, aFlags,
                             manager->soloud_spec().channels);
  aSoloud->mBackendCleanupFunc = soloud_sdl_deinit;

  SDL_BindAudioStream(manager->output_device(), manager->soloud_stream());
  SDL_ResumeAudioDevice(manager->output_device());

  return 0;
}

}  // namespace

Audio::Audio(base::WeakPtr<filesystem::Filesystem> file_reader,
             scoped_refptr<CoreConfigure> config)
    : file_reader_(file_reader), config_(config) {
  InitAudioDeviceInternal();

  if (output_device_)
    me_watcher_.reset(new std::jthread(&Audio::MeMonitorInternal, this));
}

Audio::~Audio() {
  if (!output_device_)
    return;

  quit_flag_ = true;
  me_watcher_->join();

  DestroyAudioDeviceInternal();
}

void Audio::SetupMidi() {
  // TODO: unsupport MIDI
}

void Audio::BGMPlay(const std::string& filename,
                    int volume,
                    int pitch,
                    double pos) {
  if (!output_device_) {
    LOG(WARNING) << "[Content] Warning: try to play stream on disabled device.";
    return;
  }

  if (!bgm_.source || bgm_.filename != filename) {
    bgm_.source.reset(new SoLoud::Wav());
    bgm_.filename = filename;
    file_reader_->OpenRead(
        filename,
        base::BindRepeating(
            [](SoLoud::Wav* source, SDL_IOStream* ops, const std::string& ext) {
              size_t out_size;
              uint8_t* mem = static_cast<uint8_t*>(
                  read_mem_file(ops, &out_size, SDL_TRUE));
              return source->loadMem(mem, out_size) == SoLoud::SO_NO_ERROR;
            },
            bgm_.source.get()));
    bgm_.source->setLooping(true);
    bgm_.play_handle = core_.play(*bgm_.source, volume / 100.0f);
  } else
    core_.setVolume(bgm_.play_handle, volume / 100.0f);

  core_.setRelativePlaySpeed(bgm_.play_handle, pitch / 100.0f);
  if (pos)
    core_.seek(bgm_.play_handle, pos);
}

void Audio::BGMStop() {
  if (!output_device_)
    return;

  bgm_.source.reset();
  bgm_.filename.clear();
  bgm_.play_handle = 0;
}

void Audio::BGMFade(int time) {
  if (!output_device_)
    return;

  core_.fadeVolume(bgm_.play_handle, 0, time * 0.001);
  core_.scheduleStop(bgm_.play_handle, time * 0.001);
}

double Audio::BGMPos() {
  if (!output_device_)
    return 0.0;

  return core_.getStreamPosition(bgm_.play_handle);
}

void Audio::BGSPlay(const std::string& filename,
                    int volume,
                    int pitch,
                    double pos) {
  if (!output_device_) {
    LOG(WARNING) << "[Content] Warning: try to play stream on disabled device.";
    return;
  }

  if (!bgs_.source || bgs_.filename != filename) {
    bgs_.source.reset(new SoLoud::Wav());
    bgs_.filename = filename;
    file_reader_->OpenRead(
        filename,
        base::BindRepeating(
            [](SoLoud::Wav* source, SDL_IOStream* ops, const std::string& ext) {
              size_t out_size;
              uint8_t* mem = static_cast<uint8_t*>(
                  read_mem_file(ops, &out_size, SDL_TRUE));
              return source->loadMem(mem, out_size) == SoLoud::SO_NO_ERROR;
            },
            bgs_.source.get()));
    bgs_.source->setLooping(true);
    bgs_.play_handle = core_.play(*bgs_.source, volume / 100.0f);
  } else
    core_.setVolume(bgs_.play_handle, volume / 100.0f);

  core_.setRelativePlaySpeed(bgs_.play_handle, pitch / 100.0f);
  if (pos)
    core_.seek(bgs_.play_handle, pos);
}

void Audio::BGSStop() {
  if (!output_device_)
    return;

  bgs_.source.reset();
  bgs_.filename.clear();
  bgs_.play_handle = 0;
}

void Audio::BGSFade(int time) {
  if (!output_device_)
    return;

  core_.fadeVolume(bgs_.play_handle, 0, time * 0.001);
  core_.scheduleStop(bgs_.play_handle, time * 0.001);
}

double Audio::BGSPos() {
  if (!output_device_)
    return 0.0;

  return core_.getStreamPosition(bgs_.play_handle);
}

void Audio::MEPlay(const std::string& filename, int volume, int pitch) {
  if (!output_device_) {
    LOG(WARNING) << "[Content] Warning: try to play stream on disabled device.";
    return;
  }

  if (!me_.source || me_.filename != filename) {
    me_.source.reset(new SoLoud::Wav());
    me_.filename = filename;
    file_reader_->OpenRead(
        filename,
        base::BindRepeating(
            [](SoLoud::Wav* source, SDL_IOStream* ops, const std::string& ext) {
              size_t out_size;
              uint8_t* mem = static_cast<uint8_t*>(
                  read_mem_file(ops, &out_size, SDL_TRUE));
              return source->loadMem(mem, out_size) == SoLoud::SO_NO_ERROR;
            },
            me_.source.get()));
    me_.play_handle = core_.play(*me_.source, volume / 100.0f);
  } else
    core_.setVolume(me_.play_handle, volume / 100.0f);

  core_.setRelativePlaySpeed(me_.play_handle, pitch / 100.0f);
}

void Audio::MEStop() {
  if (!output_device_)
    return;

  me_.source.reset();
  me_.filename.clear();
  me_.play_handle = 0;
}

void Audio::MEFade(int time) {
  if (!output_device_)
    return;

  core_.fadeVolume(me_.play_handle, 0, time * 0.001);
  core_.scheduleStop(me_.play_handle, time * 0.001);
}

void Audio::SEPlay(const std::string& filename, int volume, int pitch) {
  if (!output_device_)
    return;

  auto cache = se_cache_.find(filename);
  if (cache != se_cache_.end()) {
    // From cache
    auto handle = core_.play(*cache->second);
    core_.setVolume(handle, volume / 100.0f);
    core_.setRelativePlaySpeed(handle, pitch / 100.0f);
  } else {
    // Load from filesystem
    std::unique_ptr<SoLoud::Wav> source(new SoLoud::Wav());
    file_reader_->OpenRead(
        filename,
        base::BindRepeating(
            [](SoLoud::Wav* source, SDL_IOStream* ops, const std::string& ext) {
              size_t out_size;
              uint8_t* mem = static_cast<uint8_t*>(
                  read_mem_file(ops, &out_size, SDL_TRUE));
              return source->loadMem(mem, out_size) == SoLoud::SO_NO_ERROR;
            },
            source.get()));

    // Play new stream
    auto handle = core_.play(*source);
    core_.setVolume(handle, volume / 100.0f);
    core_.setRelativePlaySpeed(handle, pitch / 100.0f);
    se_cache_.insert(std::make_pair(filename, std::move(source)));
  }
}

void Audio::SEStop() {
  if (!output_device_)
    return;

  for (auto& it : se_cache_)
    core_.stopAudioSource(*it.second);
}

void Audio::Reset() {
  if (!output_device_)
    return;

  core_.stopAll();

  bgm_.source.reset();
  bgm_.filename.clear();
  bgm_.play_handle = 0;

  bgs_.source.reset();
  bgs_.filename.clear();
  bgs_.play_handle = 0;

  me_.source.reset();
  me_.filename.clear();
  me_.play_handle = 0;

  se_cache_.clear();
}

void Audio::InitAudioDeviceInternal() {
  if (config_->disable_audio()) {
    LOG(INFO) << "Disabled audio output";
    output_device_ = 0;
    return;
  }

  soloud_spec_.freq = 44100;
  soloud_spec_.format = SDL_AUDIO_F32;
  soloud_spec_.channels = 2;
  output_device_ =
      SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_OUTPUT, &soloud_spec_);

  if (!output_device_) {
    LOG(INFO) << "[Content] Failed to initialize audio device, audio module "
                 "will be disabled";
    return;
  }

  if (core_.init(soloud_sdlbackend_init, this, 0, 0) != SoLoud::SO_NO_ERROR)
    LOG(INFO) << "[Content] Failed to initialize audio core.";
}

void Audio::DestroyAudioDeviceInternal() {
  SDL_CloseAudioDevice(output_device_);
  LOG(INFO) << "[Content] Finalize audio core.";
}

void Audio::MeMonitorInternal() {
  while (!quit_flag_) {
    if (!core_.getPause(bgm_.play_handle) &&
        core_.isValidVoiceHandle(me_.play_handle)) {
      core_.setPause(bgm_.play_handle, true);
    }

    if (core_.getPause(bgm_.play_handle) &&
        !core_.isValidVoiceHandle(me_.play_handle)) {
      core_.setVolume(bgm_.play_handle, 0);
      core_.fadeVolume(bgm_.play_handle, 1.0f, 0.5);
      core_.setPause(bgm_.play_handle, false);
    }

    SDL_Delay(10);
  }
}

}  // namespace content
