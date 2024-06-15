// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_AOMDECODER_H_
#define CONTENT_PUBLIC_AOMDECODER_H_

#include "base/memory/ref_counted.h"
#include "components/filesystem/filesystem.h"
#include "content/public/bitmap.h"
#include "renderer/meta/gles2meta.h"
#include "renderer/quad/quad_drawable.h"
#include "third_party/av1player/src/player.hpp"

#include <optional>

namespace content {

struct WorkerShareData;

class AOMDecoder : public base::RefCounted<AOMDecoder>,
                   public Disposable,
                   public GraphicElement {
 public:
  enum class Type {
    Playing = 0,
    Paused,
    Stopped,
    Finished,
  };

  AOMDecoder(scoped_refptr<Graphics> host);
  ~AOMDecoder();

  AOMDecoder(const AOMDecoder&) = delete;
  AOMDecoder& operator=(const AOMDecoder&) = delete;

  uvpx::Player::LoadResult LoadVideo(const std::string& filename);
  void Update();

  uvpx::Player::VideoInfo* GetVideoInfo();

  void SetPlayState(Type state);
  Type GetPlayState();

  void Render(scoped_refptr<Bitmap> target);

 protected:
  void OnObjectDisposed() override;
  std::string_view DisposedObjectName() const override {
    return "AOM/AV1 Decoder";
  }

 private:
  static void OnAudioData(void* userPtr, float* pcm, size_t count);
  static void OnVideoFinished(void* userPtr);
  void CreateYUVInternal(const base::Vec2i& size);
  void DestroyYUVInternal();
  void UploadInternal(uvpx::Frame* yuv, bool* fence);
  void RenderInternal(renderer::TextureFrameBuffer* target);
  void UpdateYUVTexture(int width,
                        int height,
                        const Uint8* Yplane,
                        int Ypitch,
                        const Uint8* Uplane,
                        int Upitch,
                        const Uint8* Vplane,
                        int Vpitch);
  int VideoTexSubImage2D(GLenum target,
                         GLint xoffset,
                         GLint yoffset,
                         GLsizei width,
                         GLsizei height,
                         GLenum format,
                         GLenum type,
                         const GLvoid* pixels,
                         GLint pitch,
                         GLint bpp);

  filesystem::Filesystem* io_;
  std::unique_ptr<uvpx::Player> player_;
  std::unique_ptr<uvpx::Frame> frame_data_;
  uint64_t last_ticks_;
  int64_t counter_freq_;
  float frame_delta_;

  enum PlaneType { Plane_Y = 0, Plane_U, Plane_V };
  renderer::GLID<renderer::Texture> video_planes_[3];
  std::unique_ptr<renderer::QuadDrawable> video_quad_;

  SDL_AudioDeviceID audio_output_;
  SDL_AudioStream* audio_stream_;
};

}  // namespace content

#endif  //! CONTENT_PUBLIC_AOMDECODER_H_
