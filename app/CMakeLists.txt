# Main entry
cmake_minimum_required(VERSION 3.20.1)

#--------------------------------------------------------------------------------
# Player entry source
#--------------------------------------------------------------------------------

set(RGU_DEP_LIBS
  engine_base
  engine_content
  engine_ui
  binding_mri
)

add_compile_definitions(NOMINMAX)
add_executable(rgu-console rgu_main.cc resources/app.rc)
target_link_libraries(rgu-console PUBLIC ${RGU_DEP_LIBS})
