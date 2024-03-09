# Main entry
cmake_minimum_required(VERSION 3.18.0)

add_executable(rgu_player rgu_main.cc resources/app.rc)
target_link_libraries(rgu_player core_base core_content core_ui binding_mri)

execute_process(
  COMMAND "${PYTHON_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/buildtools/build_file_binary.py" "${PROJECT_SOURCE_DIR}/app/resources/rgu_favicon_64.png" -o "${RGU_AUTOGEN_DIR}/app/icon.xxd"
  RESULT_VARIABLE SHADER_GENERATE_RESULT
  OUTPUT_QUIET
)
