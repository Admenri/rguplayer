# Content Compositor
cmake_minimum_required(VERSION 3.18.0)

#--------------------------------------------------------------------------------
# Define content worker source
#--------------------------------------------------------------------------------

set(CONTENT_SRC
  binding/binding_runner.cc
  binding/binding_runner.h
  event/event_runner.cc
  event/event_runner.h
  render/render_runner.cc
  render/render_runner.h
  scheduler/worker_cc.cc
  scheduler/worker_cc.h
  script/bitmap.cc
  script/bitmap.h
  script/disposable.h
  script/drawable.cc
  script/drawable.h
  script/flashable.h
  script/graphics.cc
  script/graphics.h
  script/plane.cc
  script/plane.h
  script/sprite.cc
  script/sprite.h
  script/table.cc
  script/table.h
  script/utility.cc
  script/utility.h
  script/viewport.cc
  script/viewport.h
)

add_library(core_content ${CONTENT_SRC})
target_link_libraries(core_content core_base core_gpu SDL2::Core SDL2::Image SDL2::TTF)
