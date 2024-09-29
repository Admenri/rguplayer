# Content compositor components of RGU Kernel
cmake_minimum_required(VERSION 3.20.1)

#--------------------------------------------------------------------------------
# Content source of core
#--------------------------------------------------------------------------------

set(CORE_CONTENT_SRC
  profile/engine_profile.cc
  profile/engine_profile.h
  public/bitmap.cc
  public/bitmap.h
  public/disposable.h
  public/drawable.cc
  public/drawable.h
  public/flashable.h
  public/font.cc
  public/font.h
  public/graphics.cc
  public/graphics.h
  public/input.cc
  public/input.h
  public/mouse.cc
  public/mouse.h
  public/serializable.cc
  public/serializable.h
  public/sprite.cc
  public/sprite.h
  public/table.cc
  public/table.h
  public/touch.cc
  public/touch.h
  public/utility.cc
  public/utility.h
  public/viewport.cc
  public/viewport.h
)

add_library(engine_content PUBLIC ${CORE_CONTENT_SRC})
target_link_libraries(engine_content PUBLIC
                      engine_base
                      engine_renderer
                      engine_ui
                      components_filesystem
                      components_fpslimiter
                      sdl_image_loader
                      sdl_ttf_render)
