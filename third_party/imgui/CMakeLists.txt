# Main imgui
cmake_minimum_required(VERSION 3.18.0)

#--------------------------------------------------------------------------------
# Source of IMGUI
#--------------------------------------------------------------------------------

set(IMGUI_SOURCE
  imgui.cpp
  imgui_demo.cpp
  imgui_draw.cpp
  imgui_tables.cpp
  imgui_widgets.cpp

  imgui_impl_opengl3.cpp
  imgui_impl_sdl3.cpp
)

add_library(base_imgui ${IMGUI_SOURCE})
if(SDL_STATIC)
target_link_libraries(base_imgui PUBLIC SDL3::SDL3-static)
else()
target_link_libraries(base_imgui PUBLIC SDL3-shared)
endif()
