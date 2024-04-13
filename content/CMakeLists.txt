# Content compositor components of RGU Core
cmake_minimum_required(VERSION 3.18.0)

#--------------------------------------------------------------------------------
# Content source of core
#--------------------------------------------------------------------------------

set(CORE_CONTENT_SRC
  common/tilemap_common.cc
  common/tilemap_common.h
  common/serializable.cc
  common/serializable.h
  config/core_config.cc
  config/core_config.h
  engine/binding_engine.cc
  engine/binding_engine.h
  public/audio.cc
  public/audio.h
  public/bitmap.cc
  public/bitmap.h
  public/disposable.h
  public/drawable.cc
  public/drawable.h
  public/flashable.h
  public/font.cc
  public/font.h
  public/geometry.cc
  public/geometry.h
  public/graphics.cc
  public/graphics.h
  public/input.cc
  public/input.h
  public/mouse.cc
  public/mouse.h
  public/plane.cc
  public/plane.h
  public/shader.cc
  public/shader.h
  public/sprite.cc
  public/sprite.h
  public/table.cc
  public/table.h
  public/tilemap.cc
  public/tilemap.h
  public/tilemap2.cc
  public/tilemap2.h
  public/tilequad.h
  public/utility.cc
  public/utility.h
  public/viewport.cc
  public/viewport.h
  public/window.cc
  public/window.h
  public/window2.cc
  public/window2.h
  worker/binding_worker.cc
  worker/binding_worker.h
  worker/content_compositor.cc
  worker/content_compositor.h
  worker/content_params.h
  worker/event_runner.cc
  worker/event_runner.h
  worker/renderer_worker.cc
  worker/renderer_worker.h
)

add_library(core_content ${CORE_CONTENT_SRC})
target_link_libraries(core_content PUBLIC
                      core_base
                      core_renderer
                      components_filesystem
                      components_fpslimiter
                      SDL3_image::SDL3_image
                      SDL3_ttf::SDL3_ttf
                      SDL3_net::SDL3_net
                      soloud)

if (MSVC)
  target_compile_options(core_content PRIVATE /O2 /WX "$<$<CXX_COMPILER_ID:MSVC>:-utf-8>")
else()
  target_compile_options(core_content PRIVATE -O2 -Werror)
endif()
