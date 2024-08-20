// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "components/spinecontroller/skeleton_drawable.h"

#include "base/debug/logging.h"
#include "components/filesystem/filesystem.h"
#include "renderer/thread/thread_manager.h"

#include "SDL_image.h"

namespace {

struct OGLTexture {
  renderer::GLID<renderer::Texture> tex;
  base::Vec2i size;
  GLuint minFilter = 0;
  GLuint magFilter = 0;
  GLuint uWrap = 0;
  GLuint vWrap = 0;
};

static unsigned int ConvertTextureFilter(spine::TextureFilter filter) {
  switch (filter) {
    case spine::TextureFilter_Nearest:
      return GL_NEAREST;
    case spine::TextureFilter_Linear:
      return GL_LINEAR;
    case spine::TextureFilter_MipMapNearestNearest:
      return GL_NEAREST_MIPMAP_NEAREST;
    case spine::TextureFilter_MipMapLinearNearest:
      return GL_LINEAR_MIPMAP_NEAREST;
    case spine::TextureFilter_MipMapNearestLinear:
      return GL_NEAREST_MIPMAP_LINEAR;
    case spine::TextureFilter_MipMapLinearLinear:
      return GL_LINEAR_MIPMAP_LINEAR;
    default:
      return GL_LINEAR;
  }

  return GL_LINEAR;
}

static unsigned int ConvertTextureWrap(spine::TextureWrap wrap) {
  switch (wrap) {
    case spine::TextureWrap_MirroredRepeat:
      return GL_MIRRORED_REPEAT;
    case spine::TextureWrap_ClampToEdge:
      return GL_CLAMP_TO_EDGE;
    case spine::TextureWrap_Repeat:
      return GL_REPEAT;
  }

  return GL_REPEAT;
}

}  // namespace

namespace spinecontroller {

void OGLTextureLoader::load(spine::AtlasPage& page, const spine::String& path) {
  auto* texture = new OGLTexture();

  // Exception take to script layer
  SDL_IOStream* ops = io_->OpenReadRaw(path.buffer());

  if (ops) {
    SDL_Surface* surf = IMG_LoadTyped_IO(ops, SDL_TRUE, "PNG");
    if (surf->format->format != SDL_PIXELFORMAT_ABGR8888) {
      SDL_Surface* new_surf =
          SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_ABGR8888);
      SDL_DestroySurface(surf);
      surf = new_surf;
    }

    texture->tex = renderer::Texture::Gen();
    renderer::Texture::Bind(texture->tex);
    renderer::Texture::TexImage2D(surf->w, surf->h, GL_RGBA, surf->pixels);
    renderer::Texture::SetFilter();
    renderer::Texture::SetWrap();

    texture->minFilter = ConvertTextureFilter(page.minFilter);
    texture->magFilter = ConvertTextureFilter(page.magFilter);
    texture->uWrap = ConvertTextureWrap(page.uWrap);
    texture->vWrap = ConvertTextureWrap(page.vWrap);

    page.width = texture->size.x;
    page.height = texture->size.y;
    page.texture = texture;
  } else {
    LOG(INFO) << "[SpineController] Load texture failed: " << path.buffer();
  }
}

void OGLTextureLoader::unload(void* texture) {
  if (!texture)
    return;

  auto* tex = static_cast<OGLTexture*>(texture);
  renderer::Texture::Del(tex->tex);
  delete tex;
}

SkeletonDrawable::SkeletonDrawable(spine::SkeletonData* skeleton,
                                   spine::AnimationStateData* stateData)
    : skeleton_(std::make_unique<spine::Skeleton>(skeleton)) {
  spine::Bone::setYDown(true);
  owns_state_data_ = !!stateData;
  state_ = std::make_unique<spine::AnimationState>(
      owns_state_data_ ? stateData : new spine::AnimationStateData(skeleton));

  quad_indices_.add(0);
  quad_indices_.add(1);
  quad_indices_.add(2);
  quad_indices_.add(2);
  quad_indices_.add(3);
  quad_indices_.add(0);

  vertex_array_.vbo = renderer::VertexBuffer::Gen();
  vertex_array_.ibo = renderer::GLID<renderer::IndexBuffer>();
  renderer::VertexArray<renderer::CommonVertex>::Init(vertex_array_);
}

SkeletonDrawable::~SkeletonDrawable() {
  if (owns_state_data_)
    delete state_->getData();

  renderer::VertexArray<renderer::CommonVertex>::Uninit(vertex_array_);
  renderer::VertexBuffer::Del(vertex_array_.vbo);
}

void SkeletonDrawable::Update(float delta, spine::Physics physics) {
  state_->update(delta);
  state_->apply(*skeleton_);
  skeleton_->update(delta);
  skeleton_->updateWorldTransform(physics);
}

void SkeletonDrawable::Draw(const base::Vec2i& offset) {
  OGLTexture* texture = nullptr;
  renderer::CommonVertex vertex;
  for (unsigned i = 0; i < skeleton_->getSlots().size(); ++i) {
    spine::Slot& slot = *skeleton_->getDrawOrder()[i];
    spine::Attachment* attachment = slot.getAttachment();
    if (!attachment) {
      clipper_.clipEnd(slot);
      continue;
    }

    // Early out if the slot color is 0 or the bone is not active
    if (slot.getColor().a == 0 || !slot.getBone().isActive()) {
      clipper_.clipEnd(slot);
      continue;
    }

    spine::Vector<float>* vertices = &world_vertices_;
    int verticesCount = 0;
    spine::Vector<float>* uvs = NULL;
    spine::Vector<unsigned short>* indices;
    int indicesCount = 0;
    spine::Color* attachmentColor;

    if (attachment->getRTTI().isExactly(spine::RegionAttachment::rtti)) {
      spine::RegionAttachment* regionAttachment =
          (spine::RegionAttachment*)attachment;
      attachmentColor = &regionAttachment->getColor();

      // Early out if the slot color is 0
      if (attachmentColor->a == 0) {
        clipper_.clipEnd(slot);
        continue;
      }

      world_vertices_.setSize(8, 0);
      regionAttachment->computeWorldVertices(slot, world_vertices_, 0, 2);
      verticesCount = 4;
      uvs = &regionAttachment->getUVs();
      indices = &quad_indices_;
      indicesCount = 6;
      texture = (OGLTexture*)regionAttachment->getRegion()->rendererObject;

    } else if (attachment->getRTTI().isExactly(spine::MeshAttachment::rtti)) {
      spine::MeshAttachment* mesh = (spine::MeshAttachment*)attachment;
      attachmentColor = &mesh->getColor();

      // Early out if the slot color is 0
      if (attachmentColor->a == 0) {
        clipper_.clipEnd(slot);
        continue;
      }

      world_vertices_.setSize(mesh->getWorldVerticesLength(), 0);
      mesh->computeWorldVertices(slot, 0, mesh->getWorldVerticesLength(),
                                 world_vertices_.buffer(), 0, 2);
      verticesCount = mesh->getWorldVerticesLength() >> 1;
      uvs = &mesh->getUVs();
      indices = &mesh->getTriangles();
      indicesCount = indices->size();
      texture = (OGLTexture*)mesh->getRegion()->rendererObject;

    } else if (attachment->getRTTI().isExactly(
                   spine::ClippingAttachment::rtti)) {
      spine::ClippingAttachment* clip =
          (spine::ClippingAttachment*)slot.getAttachment();
      clipper_.clipStart(slot, clip);
      continue;
    } else
      continue;

    Uint8 r = static_cast<Uint8>(skeleton_->getColor().r * slot.getColor().r *
                                 attachmentColor->r * 255);
    Uint8 g = static_cast<Uint8>(skeleton_->getColor().g * slot.getColor().g *
                                 attachmentColor->g * 255);
    Uint8 b = static_cast<Uint8>(skeleton_->getColor().b * slot.getColor().b *
                                 attachmentColor->b * 255);
    Uint8 a = static_cast<Uint8>(skeleton_->getColor().a * slot.getColor().a *
                                 attachmentColor->a * 255);
    vertex.color.x = r;
    vertex.color.y = g;
    vertex.color.z = b;
    vertex.color.w = a;

    if (clipper_.isClipping()) {
      clipper_.clipTriangles(world_vertices_, *indices, *uvs, 2);
      vertices = &clipper_.getClippedVertices();
      verticesCount = clipper_.getClippedVertices().size() >> 1;
      uvs = &clipper_.getClippedUVs();
      indices = &clipper_.getClippedTriangles();
      indicesCount = clipper_.getClippedTriangles().size();
    }

    memset(&vertex, 0, sizeof(vertex));
    for (int ii = 0; ii < indicesCount; ++ii) {
      int index = (*indices)[ii] << 1;
      vertex.position.x = (*vertices)[index];
      vertex.position.y = (*vertices)[index + 1];
      vertex.texCoord.x = (*uvs)[index];
      vertex.texCoord.y = (*uvs)[index + 1];
      vertex_buffer_.add(vertex);
    }

    drawOGL(offset, texture, slot.getData().getBlendMode());

    clipper_.clipEnd(slot);
  }

  clipper_.clipEnd();
}

void SkeletonDrawable::drawOGL(const base::Vec2i& offset,
                               OGLTexture* texture,
                               spine::BlendMode blend_mode) {
  renderer::VertexBuffer::Bind(vertex_array_.vbo);
  renderer::VertexBuffer::BufferData(
      vertex_buffer_.size() * sizeof(renderer::CommonVertex),
      vertex_buffer_.buffer());

  auto& shader = renderer::GSM.shaders()->spine;
  shader.Bind();
  shader.SetProjectionMatrix(renderer::GSM.states.viewport.Current().Size());
  shader.SetTexture(texture->tex);
  shader.SetTransOffset(offset);

  renderer::GSM.states.blend.Push(true);
  if (!use_premultiplied_alpha_) {
    switch (blend_mode) {
      default:
      case spine::BlendMode_Normal:
        renderer::GL.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        break;
      case spine::BlendMode_Additive:
        renderer::GL.BlendFunc(GL_SRC_ALPHA, GL_ONE);
        break;
      case spine::BlendMode_Multiply:
        renderer::GL.BlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
        break;
      case spine::BlendMode_Screen:
        renderer::GL.BlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
        break;
    }
  } else {
    switch (blend_mode) {
      default:
      case spine::BlendMode_Normal:
        renderer::GL.BlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        break;
      case spine::BlendMode_Additive:
        renderer::GL.BlendFunc(GL_ONE, GL_ONE);
        break;
      case spine::BlendMode_Multiply:
        renderer::GL.BlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
        break;
      case spine::BlendMode_Screen:
        renderer::GL.BlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
        break;
    }
  }

  renderer::VertexArray<renderer::CommonVertex>::Bind(vertex_array_);
  renderer::GL.DrawArrays(GL_TRIANGLES, 0, vertex_buffer_.size());
  renderer::VertexArray<renderer::CommonVertex>::Unbind();

  renderer::GSM.states.blend.Pop();
  renderer::GSM.states.blend_func.Refresh();
}

}  // namespace spinecontroller
