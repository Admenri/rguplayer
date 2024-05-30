// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "content/public/aomdecoder.h"

#include "SDL_timer.h"

#include "renderer/quad/quad_drawable.h"
#include "third_party/av1player/src/utils.hpp"

namespace content {

AOMDecoder::AOMDecoder(scoped_refptr<Graphics> host)
    : Disposable(host),
      io_(host->filesystem()),
      last_ticks_(0),
      counter_freq_(SDL_GetPerformanceFrequency()),
      frame_delta_(0.0f),
      audio_output_(0),
      audio_stream_(nullptr) {
  uvpx::setDebugLog(
      [](const char* msg) { LOG(INFO) << "[AOMDecoder] " << msg; });
}

AOMDecoder::~AOMDecoder() {
  Dispose();
}

uvpx::Player::LoadResult AOMDecoder::LoadVideo(const std::string& filename) {
  if (player_)
    return uvpx::Player::LoadResult::AlreadyReaded;

  // Read video file
  SDL_IOStream* ops = io_->OpenReadRaw(filename);

  // Create player instance
  player_ = std::make_unique<uvpx::Player>(uvpx::Player::defaultConfig());
  auto result = player_->load(ops, 0, false);

  // Initialize timer
  last_ticks_ = SDL_GetPerformanceCounter();
  counter_freq_ = SDL_GetPerformanceFrequency();
  frame_delta_ = 1.0f / (float)player_->info().frameRate;

  // Init OGL & audio device
  if (result == uvpx::Player::LoadResult::Success) {
    player_->setOnAudioData(OnAudioData, this);
    player_->setOnVideoFinished(OnVideoFinished, this);

    // Init yuv texture
    auto info = player_->info();
    CreateYUVInternal(info.width, info.height);

    // Init Audio components
    SDL_AudioSpec wanted_spec;
    wanted_spec.freq = info.audioFrequency;
    wanted_spec.format = SDL_AUDIO_F32;
    wanted_spec.channels = info.audioChannels;

    audio_output_ =
        SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_OUTPUT, &wanted_spec);
    if (audio_output_) {
      audio_stream_ = SDL_CreateAudioStream(&wanted_spec, &wanted_spec);
      SDL_BindAudioStream(audio_output_, audio_stream_);
    }
  }

  return result;
}

void AOMDecoder::Update() {
  if (!player_)
    return;

  // Update frame delta
  player_->update(frame_delta_);

  // Update timer
  uint64_t current_ticks = SDL_GetPerformanceCounter();
  frame_delta_ =
      static_cast<float>(current_ticks - last_ticks_) / counter_freq_;
  last_ticks_ = current_ticks;
}

uvpx::Player::VideoInfo AOMDecoder::GetVideoInfo() {
  if (!player_)
    return uvpx::Player::VideoInfo{0};

  return player_->info();
}

void AOMDecoder::SetPlayState(Type state) {
  if (!player_)
    return;

  switch (state) {
    case content::AOMDecoder::Type::Playing:
      player_->play();
      if (audio_output_)
        SDL_ResumeAudioDevice(audio_output_);
      break;
    case content::AOMDecoder::Type::Paused:
      player_->pause();
      if (audio_output_)
        SDL_PauseAudioDevice(audio_output_);
      break;
    case content::AOMDecoder::Type::Stopped:
      player_->stop();
      break;
    default:
      break;
  }
}

AOMDecoder::Type AOMDecoder::GetPlayState() {
  if (!player_)
    return Type::Stopped;

  if (player_->isPlaying())
    return Type::Playing;
  else if (player_->isPaused())
    return Type::Paused;
  else if (player_->isStopped())
    return Type::Stopped;
  else if (player_->isFinished())
    return Type::Finished;
  else
    return Type();
}

void AOMDecoder::Render(scoped_refptr<Bitmap> target) {
  if (!player_ || !target)
    return;

  uvpx::Frame* yuv = nullptr;
  auto info = player_->info();
  auto canvas_size = target->GetSize();
  if ((yuv = player_->lockRead()) != nullptr) {
    UpdateYUVTexture(info.width, info.height, yuv->y(), yuv->yPitch(), yuv->u(),
                     yuv->uvPitch(), yuv->v(), yuv->uvPitch());
    player_->unlockRead();

    auto frame_size = base::Vec2i(info.width, info.height);
    auto& shader = renderer::GSM.shaders()->yuv;
    shader.Bind();
    shader.SetProjectionMatrix(canvas_size);
    shader.SetTransOffset(base::Vec2());
    shader.SetTextureSize(frame_size);
    shader.SetTextureY(video_planes_[Plane_Y]);
    shader.SetTextureU(video_planes_[Plane_U]);
    shader.SetTextureV(video_planes_[Plane_V]);

    auto& tfb = target->GetTexture();
    renderer::FrameBuffer::Bind(tfb.fbo);
    renderer::FrameBuffer::ClearColor();
    renderer::FrameBuffer::Clear();

    renderer::GSM.states.viewport.Push(canvas_size);
    renderer::GSM.states.blend.Push(false);

    auto* quad = renderer::GSM.common_quad();
    quad->SetPositionRect(base::Rect(canvas_size));
    quad->SetTexCoordRect(base::Rect(frame_size));
    quad->Draw();

    renderer::GSM.states.blend.Pop();
    renderer::GSM.states.viewport.Pop();
  }
}

void AOMDecoder::OnObjectDisposed() {
  if (audio_stream_)
    SDL_DestroyAudioStream(audio_stream_);

  if (audio_output_)
    SDL_CloseAudioDevice(audio_output_);

  DestroyYUVInternal();

  player_.reset();
}

void AOMDecoder::OnAudioData(void* userPtr, float* pcm, size_t count) {
  auto* self = static_cast<AOMDecoder*>(userPtr);
  SDL_PutAudioStreamData(self->audio_stream_, pcm, count * sizeof(float));
}

void AOMDecoder::OnVideoFinished(void* userPtr) {
  auto* self = static_cast<AOMDecoder*>(userPtr);
  uvpx::debugLog("instace: %p video play finished.", self);
}

void AOMDecoder::CreateYUVInternal(int width, int height) {
  if (video_quad_)
    return;

  auto gen_plane = [](renderer::GLID<renderer::Texture>* tex, GLsizei w,
                      GLsizei h) {
    *tex = renderer::Texture::Gen();
    renderer::Texture::Bind(*tex);
    renderer::Texture::SetFilter(GL_LINEAR);
    renderer::Texture::SetWrap();
    renderer::Texture::TexImage2D(w, h, GL_LUMINANCE);
  };

  gen_plane(&video_planes_[Plane_Y], width, height);
  gen_plane(&video_planes_[Plane_U], (width + 1) / 2, (height + 1) / 2);
  gen_plane(&video_planes_[Plane_V], (width + 1) / 2, (height + 1) / 2);

  video_quad_ = std::make_unique<renderer::QuadDrawable>();
}

void AOMDecoder::DestroyYUVInternal() {
  if (!video_quad_)
    return;

  renderer::Texture::Del(video_planes_[Plane_Y]);
  renderer::Texture::Del(video_planes_[Plane_U]);
  renderer::Texture::Del(video_planes_[Plane_V]);
}

void AOMDecoder::UpdateYUVTexture(int width,
                                  int height,
                                  const Uint8* Yplane,
                                  int Ypitch,
                                  const Uint8* Uplane,
                                  int Upitch,
                                  const Uint8* Vplane,
                                  int Vpitch) {
  renderer::Texture::Bind(video_planes_[Plane_V]);
  VideoTexSubImage2D(GL_TEXTURE_2D, 0, 0, (width + 1) / 2, (height + 1) / 2,
                     GL_LUMINANCE, GL_UNSIGNED_BYTE, Vplane, Vpitch, 1);

  renderer::Texture::Bind(video_planes_[Plane_U]);
  VideoTexSubImage2D(GL_TEXTURE_2D, 0, 0, (width + 1) / 2, (height + 1) / 2,
                     GL_LUMINANCE, GL_UNSIGNED_BYTE, Uplane, Upitch, 1);

  renderer::Texture::Bind(video_planes_[Plane_Y]);
  VideoTexSubImage2D(GL_TEXTURE_2D, 0, 0, width, height, GL_LUMINANCE,
                     GL_UNSIGNED_BYTE, Yplane, Ypitch, 1);
}

int AOMDecoder::VideoTexSubImage2D(GLenum target,
                                   GLint xoffset,
                                   GLint yoffset,
                                   GLsizei width,
                                   GLsizei height,
                                   GLenum format,
                                   GLenum type,
                                   const GLvoid* pixels,
                                   GLint pitch,
                                   GLint bpp) {
  Uint8* blob = NULL;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  Uint32* blob2 = NULL;
#endif
  Uint8* src;
  int src_pitch;
  int y;

  if ((width == 0) || (height == 0) || (bpp == 0)) {
    return 0; /* nothing to do */
  }

  /* Reformat the texture data into a tightly packed array */
  src_pitch = width * bpp;
  src = (Uint8*)pixels;
  if (pitch != src_pitch) {
    blob = (Uint8*)SDL_malloc(src_pitch * height);
    if (!blob) {
      return SDL_OutOfMemory();
    }
    src = blob;
    for (y = 0; y < height; ++y) {
      SDL_memcpy(src, pixels, src_pitch);
      src += src_pitch;
      pixels = (Uint8*)pixels + pitch;
    }
    src = blob;
  }

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  if (format == GL_RGBA) {
    int i;
    Uint32* src32 = (Uint32*)src;
    blob2 = (Uint32*)SDL_malloc(src_pitch * height);
    if (!blob2) {
      if (blob) {
        SDL_free(blob);
      }
      return SDL_OutOfMemory();
    }
    for (i = 0; i < (src_pitch * height) / 4; i++) {
      blob2[i] = SDL_Swap32(src32[i]);
    }
    src = (Uint8*)blob2;
  }
#endif

  renderer::GL.TexSubImage2D(target, 0, xoffset, yoffset, width, height, format,
                             type, src);

  if (blob) {
    SDL_free(blob);
  }
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  if (blob2) {
    SDL_free(blob2);
  }
#endif
  return 0;
}

}  // namespace content
