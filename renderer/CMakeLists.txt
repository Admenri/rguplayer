# Renderer part of engine
cmake_minimum_required(VERSION 3.18.0)

#--------------------------------------------------------------------------------
# Renderer source
#--------------------------------------------------------------------------------

set(RENDERER_SRC
  buffer/vertex_buffer.cc
  buffer/vertex_buffer.h
  compositor/renderer_cc.cc
  compositor/renderer_cc.h
  paint/frame_buffer_canvas.cc
  paint/frame_buffer_canvas.h
  paint/quad_draw.cc
  paint/quad_draw.h
  state/state_stack.cc
  state/state_stack.h
  renderer_utility.h
)

add_library(core_renderer ${RENDERER_SRC})
target_link_libraries(core_renderer core_base core_gpu SDL2::Core)
