# App wrapper
cmake_minimum_required(VERSION 3.18.0)

add_executable(player rgu_main.cc resources/app.rc)
target_link_libraries(player SDL2::Core SDL2::TTF SDL2::Image core_base core_gpu core_renderer core_ui core_content core_modules)
