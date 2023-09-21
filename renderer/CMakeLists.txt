# Renderer part of engine
cmake_minimum_required(VERSION 3.18.0)

#--------------------------------------------------------------------------------
# Renderer source
#--------------------------------------------------------------------------------

set(RENDERER_SRC
  buffer_wrapper/gl_vertex.cc
  buffer_wrapper/gl_vertex.h
  compositor/gl_cc.cc
  compositor/gl_cc.h
  ogl/gl_frame_buffer_texture.cc
  ogl/gl_frame_buffer_texture.h
  ogl/gl_shader_manager.cc
  ogl/gl_shader_manager.h
  ogl/gl_statestack.cc
  ogl/gl_statestack.h
  quad_draw.cc
  quad_draw.h
  renderer_utility.h
)

add_library(core_renderer ${RENDERER_SRC})
target_link_libraries(core_renderer core_base core_gpu SDL2::Core)
