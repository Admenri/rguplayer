## IMGUI
cmake_minimum_required(VERSION 3.20.1)

#--------------------------------------------------------------------------------
# Source
#--------------------------------------------------------------------------------

set(IMGUI_SOURCE
  imgui.cpp
  imgui_demo.cpp
  imgui_draw.cpp
  imgui_impl_bgfx.cpp
  imgui_impl_sdl3.cpp
  imgui_tables.cpp
  imgui_widgets.cpp
)

add_library(bgfx_imgui STATIC ${IMGUI_SOURCE})
target_link_libraries(bgfx_imgui PUBLIC engine_base bgfx bx bimg)
