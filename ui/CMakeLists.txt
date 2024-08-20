# UI Library
cmake_minimum_required(VERSION 3.20.1)

#--------------------------------------------------------------------------------
# Define ui source
#--------------------------------------------------------------------------------

set(UI_SRC
  widget/widget.cc
  widget/widget.h
)

add_library(core_ui ${UI_SRC})
target_link_libraries(core_ui PUBLIC core_base)
