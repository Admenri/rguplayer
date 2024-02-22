# Components - settingss menu
cmake_minimum_required(VERSION 3.18.0)

#--------------------------------------------------------------------------------
# Source of menu
#--------------------------------------------------------------------------------

set(COMPONENTS_SETTINGSMENU_SRC
  settings_menu.cc
  settings_menu.h
)

add_library(components_settingsmenu ${COMPONENTS_SETTINGSMENU_SRC})
target_link_libraries(components_settingsmenu core_base)
