# Renderer part of engine
cmake_minimum_required(VERSION 3.18.0)

#--------------------------------------------------------------------------------
# Renderer source
#--------------------------------------------------------------------------------

set(RENDERER_SRC
  ogl/gl_shader_manager.cc
  ogl/gl_shader_manager.h
  ogl/gl_statestack.cc
  ogl/gl_statestack.h
  renderer_utility.h
)

add_library(core_renderer ${RENDERER_SRC})
target_link_libraries(core_renderer core_base core_gpu SDL2::Core)
