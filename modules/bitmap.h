#ifndef MODULES_BITMAP_H_
#define MODULES_BITMAP_H_

#include <SDL_surface.h>

#include <memory>
#include <string>

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
  base::Vec4i GetRect();
  void Clear();

  SDL_Surface* SaveSurface();

  // Called from RenderThread
  GLuint GetTexture() { return texture_->GetTextureRaw(); }

 protected:
  void OnObjectDisposed() override;

 private:
  void InitBufferWH(int width, int height);
  void InitBufferPath(const std::string& filename);
  void SaveSurfaceInternal(SDL_Surface* tmp_surf, base::OnceClosure complete);

  void EnsureSurfaceFormat(SDL_Surface*& surface);

  scoped_refptr<content::RendererThread> worker_;
  scoped_refptr<renderer::GLTexture> texture_;

  SDL_PixelFormat* format_ = nullptr;

  base::WeakPtrFactory<Bitmap> weak_ptr_factory_{this};
};

}  // namespace modules

#endif  // MODULES_BITMAP_H_