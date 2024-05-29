// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "content/public/aomdecoder.h"

#include "SDL_timer.h"

#include "renderer/quad/quad_drawable.h"

namespace content {

AOMDecoder::AOMDecoder(scoped_refptr<Graphics> host,
                       WorkerShareData* share_data)
    : Disposable(host),
      io_(host->filesystem()),
      last_ticks_(0),
      counter_freq_(SDL_GetPerformanceFrequency()),
      frame_delta_(0.0f) {
  audio_output_ = share_data->output_device;
  player_ = std::make_unique<uvpx::Player>(uvpx::Player::defaultConfig());
}

AOMDecoder::~AOMDecoder() {
  Dispose();
}

uvpx::Player::LoadResult AOMDecoder::LoadVideo(const std::string& filename) {
  SDL_IOStream* ops = io_->OpenReadRaw(filename);
  auto result = player_->load(ops, 0, false);

  last_ticks_ = SDL_GetPerformanceCounter();
  counter_freq_ = SDL_GetPerformanceFrequency();
  frame_delta_ = 1.0f / (float)player_->info().frameRate;

  if (result == uvpx::Player::LoadResult::Success) {
    player_->setOnAudioData(OnAudioData, this);
    auto info = player_->info();
    CreateYUVInternal(info.width, info.height);
  }

  return result;
}

void AOMDecoder::Update() {
  if (!player_)
    return;

  player_->update(frame_delta_);

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
      break;
    case content::AOMDecoder::Type::Paused:
      player_->pause();
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
  else
    return Type();
}

void AOMDecoder::Render(scoped_refptr<Bitmap> target) {
  if (!player_ || !target)
    return;

  auto canvas_size = target->GetSize();
  uvpx::Frame* yuv = nullptr;
  auto info = player_->info();
  if ((yuv = player_->lockRead()) != nullptr) {
    UpdateYUVTexture(info.width, info.height, yuv->y(), yuv->yPitch(), yuv->u(),
                     yuv->uvPitch(), yuv->v(), yuv->uvPitch());

    player_->unlockRead();
  }

  if (yuv) {
    auto& shader = renderer::GSM.shaders()->yuv;
    shader.Bind();
    shader.SetProjectionMatrix(canvas_size);
    shader.SetTransOffset(base::Vec2());
    shader.SetTextureSize(canvas_size);
    shader.SetTextureY(plane_y_);
    shader.SetTextureU(plane_u_);
    shader.SetTextureV(plane_v_);

    renderer::GSM.states.viewport.Push(canvas_size);

    auto& tfb = target->GetTexture();
    renderer::FrameBuffer::Bind(tfb.fbo);
    renderer::FrameBuffer::ClearColor();
    renderer::FrameBuffer::Clear();

    auto* quad = renderer::GSM.common_quad();
    renderer::GSM.states.blend.Push(false);
    quad->SetPositionRect(base::Rect(canvas_size));
    quad->SetTexCoordRect(base::Rect(canvas_size));
    quad->Draw();
    renderer::GSM.states.blend.Pop();

    renderer::GSM.states.viewport.Pop();
  }
}

void AOMDecoder::OnObjectDisposed() {
  DestroyYUVInternal();
  player_.reset();
}

void AOMDecoder::OnAudioData(void* userPtr, float* pcm, size_t count) {
  [[maybe_unused]] auto* self = static_cast<AOMDecoder*>(userPtr);
}

void AOMDecoder::CreateYUVInternal(int width, int height) {
  plane_y_ = renderer::Texture::Gen();
  renderer::Texture::Bind(plane_y_);
  renderer::Texture::SetWrap();
  renderer::Texture::SetFilter();
  renderer::Texture::TexImage2D(width, height, GL_LUMINANCE);

  plane_u_ = renderer::Texture::Gen();
  renderer::Texture::Bind(plane_u_);
  renderer::Texture::SetWrap();
  renderer::Texture::SetFilter();
  renderer::Texture::TexImage2D((width + 1) / 2, (height + 1) / 2,
                                GL_LUMINANCE);

  plane_v_ = renderer::Texture::Gen();
  renderer::Texture::Bind(plane_v_);
  renderer::Texture::SetWrap();
  renderer::Texture::SetFilter();
  renderer::Texture::TexImage2D((width + 1) / 2, (height + 1) / 2,
                                GL_LUMINANCE);
}

void AOMDecoder::DestroyYUVInternal() {
  renderer::Texture::Del(plane_y_);
  renderer::Texture::Del(plane_u_);
  renderer::Texture::Del(plane_v_);
}

void AOMDecoder::UpdateYUVTexture(int width,
                                  int height,
                                  const Uint8* Yplane,
                                  int Ypitch,
                                  const Uint8* Uplane,
                                  int Upitch,
                                  const Uint8* Vplane,
                                  int Vpitch) {
  renderer::Texture::Bind(plane_v_);
  VideoTexSubImage2D(GL_TEXTURE_2D, 0, 0, (width + 1) / 2, (height + 1) / 2,
                     GL_LUMINANCE, GL_UNSIGNED_BYTE, Vplane, Vpitch, 1);

  renderer::Texture::Bind(plane_u_);
  VideoTexSubImage2D(GL_TEXTURE_2D, 0, 0, (width + 1) / 2, (height + 1) / 2,
                     GL_LUMINANCE, GL_UNSIGNED_BYTE, Uplane, Upitch, 1);

  renderer::Texture::Bind(plane_y_);
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

  renderer::GL.TexImage2D(target, 0, xoffset, yoffset, width, height, format,
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
