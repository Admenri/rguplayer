# Content Compositor
cmake_minimum_required(VERSION 3.18.0)

#--------------------------------------------------------------------------------
# Define content worker source
#--------------------------------------------------------------------------------

set(CONTENT_SRC
  render_thread.cc
  render_thread.h
)

add_library(core_content ${CONTENT_SRC})
target_link_libraries(core_content core_base core_renderer core_gpu SDL2::Core)
