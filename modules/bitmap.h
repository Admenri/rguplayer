#ifndef MODULES_BITMAP_H_
#define MODULES_BITMAP_H_

#include <SDL_surface.h>

#include <memory>
#include <string>
#include <variant>

#include "base/bind/callback.h"
#include "base/math/math.h"
#include "content/render_thread.h"
#include "modules/disposable.h"
#include "renderer/compositor/renderer_cc.h"
#include "renderer/paint/frame_buffer_canvas.h"

namespace modules {

class Bitmap : public Disposable {
 public:
  Bitmap(scoped_refptr<content::RendererThread> worker, int width, int height);
  Bitmap(scoped_refptr<content::RendererThread> worker,
         const std::string& filename);
  ~Bitmap() override;

  Bitmap(const Bitmap&) = delete;
  Bitmap& operator=(const Bitmap&) = delete;

  Bitmap(Bitmap&&) = default;

  base::Vec2i GetSize();
  base::Rect GetRect();
  void Clear();

  /* Called from render thread */
  scoped_refptr<renderer::GLTexture> GetGLTexture() { return texture_; }

 protected:
  void OnObjectDisposed() override;

 private:
  void EnsureSurfaceFormat(SDL_Surface*& surface);
  void InitTextureFrameBuffer(
      std::variant<base::Vec2i, SDL_Surface*> init_data);
  void ClearInternal();

  scoped_refptr<content::RendererThread> worker_;
  scoped_refptr<renderer::GLTexture> texture_;
  std::unique_ptr<renderer::GLFrameBuffer> frame_buffer_;

  base::WeakPtrFactory<Bitmap> weak_ptr_factory_{this};
};

}  // namespace modules

#endif  // MODULES_BITMAP_H_