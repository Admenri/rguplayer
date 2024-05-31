// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_SPINECONTROLLER_SKELETON_DRAWABLE_H_
#define COMPONENTS_SPINECONTROLLER_SKELETON_DRAWABLE_H_

#include "base/math/math.h"
#include "renderer/context/gles2_context.h"
#include "renderer/meta/gles2meta.h"
#include "renderer/vertex/vertex_set.h"
#include "spine/spine.h"

namespace filesystem {
class Filesystem;
}

namespace spinecontroller {

class OGLTexture;

class OGLTextureLoader : public spine::TextureLoader {
 public:
  OGLTextureLoader(filesystem::Filesystem* io) : io_(io) {}

  OGLTextureLoader(const OGLTextureLoader&) = delete;
  OGLTextureLoader& operator=(const OGLTextureLoader&) = delete;

 protected:
  void load(spine::AtlasPage& page, const spine::String& path) override;
  void unload(void* texture) override;

 private:
  filesystem::Filesystem* io_;
};

class SkeletonDrawable {
 public:
  SkeletonDrawable(spine::SkeletonData* skeleton,
                   spine::AnimationStateData* stateData = nullptr);
  ~SkeletonDrawable();

  SkeletonDrawable(const SkeletonDrawable&) = delete;
  SkeletonDrawable& operator=(const SkeletonDrawable&) = delete;

  void Update(float delta, spine::Physics physics);
  void Draw(const base::Vec2i& offset);

  spine::Skeleton* skeleton() { return skeleton_.get(); }
  spine::AnimationState* animation_state() { return state_.get(); }
  bool& use_premultiplied_alpha() { return use_premultiplied_alpha_; }

 private:
  void drawOGL(const base::Vec2i& offset,
               OGLTexture* texture,
               spine::BlendMode blend_mode);

  std::unique_ptr<spine::Skeleton> skeleton_;
  std::unique_ptr<spine::AnimationState> state_;
  bool owns_state_data_;
  spine::Vector<unsigned short> quad_indices_;
  spine::SkeletonClipping clipper_;
  spine::Vector<float> world_vertices_;
  spine::Vector<renderer::CommonVertex> vertex_buffer_;
  bool use_premultiplied_alpha_;

  renderer::VertexArray<renderer::CommonVertex> vertex_array_;
};

}  // namespace spinecontroller

#endif  //! COMPONENTS_SPINECONTROLLER_SKELETON_DRAWABLE_H_
