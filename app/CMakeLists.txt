# Main entry
cmake_minimum_required(VERSION 3.18.0)

set(RGU_DEP_LIBS
  core_base
  core_content
  core_ui
  binding_mri
)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  add_compile_definitions(NOMINMAX)

  add_executable(rgu-console rgu_main.cc resources/app.rc)
  target_link_libraries(rgu-console PUBLIC ${RGU_DEP_LIBS})

  add_executable(rgu WIN32 rgu_main.cc resources/app.rc)
  target_link_libraries(rgu PUBLIC ${RGU_DEP_LIBS})
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  add_executable(rgu rgu_main.cc)
  target_link_libraries(rgu PUBLIC ${RGU_DEP_LIBS})
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
  add_library(main SHARED rgu_main.cc)
  target_link_libraries(main PUBLIC ${RGU_DEP_LIBS})
endif()

execute_process(
  COMMAND "${PYTHON_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/buildtools/build_file_binary.py" "${PROJECT_SOURCE_DIR}/app/resources/rgu_favicon_64.png" -o "${RGU_AUTOGEN_DIR}/app/icon.xxd"
  RESULT_VARIABLE SHADER_GENERATE_RESULT
  OUTPUT_QUIET
)
