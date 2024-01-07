# Content compositor components of RGU Core
cmake_minimum_required(VERSION 3.18.0)

#--------------------------------------------------------------------------------
# Content source of core
#--------------------------------------------------------------------------------

set(CORE_CONTENT_SRC
  public/bitmap.cc
  public/bitmap.h
  public/disposable.h
  public/drawable.cc
  public/drawable.h
  public/flashable.h
  public/font.cc
  public/font.h
  public/graphics.cc
  public/graphics.h
  public/plane.cc
  public/plane.h
  public/sprite.cc
  public/sprite.h
  public/table.cc
  public/table.h
  public/tilemap.cc
  public/tilemap.h
  public/tilequad.h
  public/utility.h
  public/viewport.cc
  public/viewport.h
  public/window.cc
  public/window.h
  worker/binding_worker.cc
  worker/binding_worker.h
  worker/content_compositor.cc
  worker/content_compositor.h
  worker/event_runner.cc
  worker/event_runner.h
  worker/renderer_worker.cc
  worker/renderer_worker.h
)

add_library(core_content ${CORE_CONTENT_SRC})
target_link_libraries(core_content core_base core_renderer SDL3::SDL3-static)
