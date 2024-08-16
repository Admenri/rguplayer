// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "content/public/aomdecoder.h"

#include "SDL_timer.h"

#include "renderer/quad/quad_drawable.h"
#include "third_party/av1player/src/utils.hpp"

namespace content {

AOMDecoder::AOMDecoder(scoped_refptr<Graphics> host)
    : Disposable(host.get()),
      GraphicElement(host),
      io_(host->filesystem()),
      last_ticks_(0),
      counter_freq_(SDL_GetPerformanceFrequency()),
      frame_delta_(0.0f),
      audio_output_(0),
      audio_stream_(nullptr) {}

AOMDecoder::~AOMDecoder() {
  Dispose();
}

uvpx::Player::LoadResult AOMDecoder::LoadVideo(const std::string& filename) {
  if (player_)
    return uvpx::Player::LoadResult::AlreadyReaded;

  // Read video file
  SDL_IOStream* ops = nullptr;
  try {
    ops = io_->OpenReadRaw(filename);
  } catch (base::Exception& e) {
    LOG(INFO) << "[AOMDecoder] Video file \"" << filename
              << "\" was not found.";
    return uvpx::Player::LoadResult::FileNotExists;
  }

  // Create player instance
  player_ = std::make_unique<uvpx::Player>(uvpx::Player::defaultConfig());
  auto result = player_->load(ops, 0, false);

  // Initialize timer
  last_ticks_ = SDL_GetPerformanceCounter();
  counter_freq_ = SDL_GetPerformanceFrequency();
  frame_delta_ = 1.0f / (float)player_->info()->frameRate;

  // Init OGL & audio device
  if (result == uvpx::Player::LoadResult::Success) {
    player_->setOnAudioData(OnAudioData, this);
    player_->setOnVideoFinished(OnVideoFinished, this);

    // Extract video info
    auto& info = *player_->info();

    // Init Audio components
    SDL_AudioSpec wanted_spec;
    wanted_spec.freq = info.audioFrequency;
    wanted_spec.format = SDL_AUDIO_F32;
    wanted_spec.channels = info.audioChannels;

    audio_output_ =
        SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &wanted_spec);
    if (audio_output_) {
      audio_stream_ = SDL_CreateAudioStream(&wanted_spec, &wanted_spec);
      SDL_BindAudioStream(audio_output_, audio_stream_);
    }

    // Init yuv texture
    frame_data_ = std::make_unique<uvpx::Frame>();
    screen()->renderer()->PostTask(
        base::BindOnce(&AOMDecoder::CreateYUVInternal, base::Unretained(this),
                       base::Vec2i(info.width, info.height)));
    screen()->renderer()->WaitForSync();
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

uvpx::Player::VideoInfo* AOMDecoder::GetVideoInfo() {
  if (!player_)
    return nullptr;

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
  if ((yuv = player_->lockRead()) != nullptr) {
    yuv->copyData(frame_data_.get());
    player_->unlockRead();

    bool sync_fence = false;
    screen()->renderer()->PostTask(
        base::BindOnce(&AOMDecoder::UploadInternal, base::Unretained(this),
                       frame_data_.get(), &sync_fence));
    while (!sync_fence)
      std::this_thread::sleep_for(std::chrono::milliseconds(1));

    screen()->renderer()->PostTask(base::BindOnce(
        &AOMDecoder::RenderInternal, base::Unretained(this), target->GetRaw()));
    screen()->renderer()->WaitForSync();
  }
}

void AOMDecoder::OnObjectDisposed() {
  if (audio_stream_)
    SDL_DestroyAudioStream(audio_stream_);

  if (audio_output_)
    SDL_CloseAudioDevice(audio_output_);

  player_.reset();

  screen()->renderer()->PostTask(
      base::BindOnce(&AOMDecoder::DestroyYUVInternal, base::Unretained(this)));
  screen()->renderer()->WaitForSync();
}

void AOMDecoder::OnAudioData(void* userPtr, float* pcm, size_t count) {
  auto* self = static_cast<AOMDecoder*>(userPtr);
  SDL_PutAudioStreamData(self->audio_stream_, pcm, count * sizeof(float));
}

void AOMDecoder::OnVideoFinished(void* userPtr) {
  auto* self = static_cast<AOMDecoder*>(userPtr);
  uvpx::debugLog("instace: %p video play finished.", self);
}

void AOMDecoder::CreateYUVInternal(const base::Vec2i& size) {
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

  gen_plane(&video_planes_[Plane_Y], size.x, size.y);
  gen_plane(&video_planes_[Plane_U], (size.x + 1) / 2, (size.y + 1) / 2);
  gen_plane(&video_planes_[Plane_V], (size.x + 1) / 2, (size.y + 1) / 2);

  video_quad_ = std::make_unique<renderer::QuadDrawable>();
}

void AOMDecoder::DestroyYUVInternal() {
  if (!video_quad_)
    return;

  renderer::Texture::Del(video_planes_[Plane_Y]);
  renderer::Texture::Del(video_planes_[Plane_U]);
  renderer::Texture::Del(video_planes_[Plane_V]);

  video_quad_.reset();
}

void AOMDecoder::UploadInternal(uvpx::Frame* yuv, bool* fence) {
  for (int i = 0; i < 3; ++i) {
    renderer::Texture::Bind(video_planes_[i]);
    VideoTexSubImage2D(GL_TEXTURE_2D, yuv->width(i), yuv->height(i),
                       GL_LUMINANCE, GL_UNSIGNED_BYTE, yuv->plane(i),
                       yuv->width(i), 1);
  }

  frame_size_ = base::Vec2i(yuv->width(0), yuv->height(0));
  *fence = true;
}

void AOMDecoder::RenderInternal(renderer::TextureFrameBuffer* target) {
  auto& info = *player_->info();
  auto& canvas_size = target->size;

  auto& shader = renderer::GSM.shaders()->yuv;
  shader.Bind();
  shader.SetProjectionMatrix(canvas_size);
  shader.SetTransOffset(base::Vec2());
  shader.SetTextureSize(frame_size_);
  shader.SetTextureY(video_planes_[Plane_Y]);
  shader.SetTextureU(video_planes_[Plane_U]);
  shader.SetTextureV(video_planes_[Plane_V]);

  auto& tfb = *target;
  renderer::FrameBuffer::Bind(tfb.fbo);
  renderer::FrameBuffer::ClearColor();
  renderer::FrameBuffer::Clear();

  renderer::GSM.states.viewport.Push(canvas_size);
  renderer::GSM.states.blend.Push(false);

  auto frame_size = base::Vec2i(info.width, info.height);
  auto* quad = renderer::GSM.common_quad();
  quad->SetPositionRect(base::Rect(canvas_size));
  quad->SetTexCoordRect(base::Rect(frame_size));
  quad->Draw();

  renderer::GSM.states.blend.Pop();
  renderer::GSM.states.viewport.Pop();
}

int AOMDecoder::VideoTexSubImage2D(GLenum target,
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

  renderer::GL.TexImage2D(target, 0, format, width, height, 0, format, type,
                          src);

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
