# Main imgui
cmake_minimum_required(VERSION 3.18.0)

#--------------------------------------------------------------------------------
# Source of IMGUI
#--------------------------------------------------------------------------------

set(IMGUI_SOURCE
  imconfig.h
  imgui.cpp
  imgui.h
  imgui_demo.cpp
  imgui_draw.cpp
  imgui_tables.cpp
  imgui_widgets.cpp
  imgui_internal.h
  imstb_rectpack.h
  imstb_textedit.h
  imstb_truetype.h

  imgui_impl_opengl3.cpp
  imgui_impl_opengl3.h
  imgui_impl_opengl3_loader.h
  imgui_impl_sdl3.cpp
  imgui_impl_sdl3.h
)

add_library(base_imgui ${IMGUI_SOURCE})
target_link_libraries(base_imgui PRIVATE core_renderer)
