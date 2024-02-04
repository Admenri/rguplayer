# Main entry
cmake_minimum_required(VERSION 3.18.0)

add_executable(rgu_player rgu_main.cc resources/app.rc)
target_link_libraries(rgu_player core_base core_content core_ui binding_mri)
