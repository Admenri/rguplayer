# Components - spine controller
cmake_minimum_required(VERSION 3.20.1)

#--------------------------------------------------------------------------------
# Source of spine rt
#--------------------------------------------------------------------------------

set(COMPONENTS_SPINECONTROLLER_SRC
  skeleton_drawable.cc
  skeleton_drawable.h
)

add_library(components_spinecontroller ${COMPONENTS_SPINECONTROLLER_SRC})
target_link_libraries(components_spinecontroller core_base core_renderer spine-cpp components_filesystem)
