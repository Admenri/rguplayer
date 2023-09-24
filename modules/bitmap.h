#ifndef MODULES_BITMAP_H_
#define MODULES_BITMAP_H_

#include <SDL_surface.h>

#include <memory>
#include <string>

#include "base/math/math.h"
#include "modules/disposable.h"
#include "renderer/paint/frame_buffer_canvas.h"

namespace modules {

class Bitmap : public Disposable {
 public:
  Bitmap(int width, int height);
  Bitmap(const std::string& filename);

  Bitmap(const Bitmap&) = delete;
  Bitmap& operator=(const Bitmap&) = delete;

  Bitmap(Bitmap&&) = default;

  void Clear();

  void Bind();
  void Unbind();

 private:
  void InitCanvas();
  void OnObjectDisposed() override;
  void EnsureSurfaceFormat(SDL_Surface*& surface);

  std::unique_ptr<renderer::FrameBufferTexture> frame_canvas_;
};

}  // namespace modules

#endif  // MODULES_BITMAP_H_