set(LIGHTWF_SRC
    cplusplus/cocos2dx/lwf_cocos2dx_bitmap.cpp
    cplusplus/cocos2dx/lwf_cocos2dx_factory.cpp
    cplusplus/cocos2dx/lwf_cocos2dx_node.cpp
    cplusplus/cocos2dx/lwf_cocos2dx_particle.cpp
    cplusplus/cocos2dx/lwf_cocos2dx_resourcecache.cpp
    cplusplus/cocos2dx/lwf_cocos2dx_textbmfont.cpp
    cplusplus/cocos2dx/lwf_cocos2dx_textttf.cpp
    cplusplus/core/lwf_animation.cpp
    cplusplus/core/lwf_bitmap.cpp
    cplusplus/core/lwf_bitmapex.cpp
    cplusplus/core/lwf_button.cpp
    cplusplus/core/lwf_core.cpp
    cplusplus/core/lwf_coredata.cpp
    cplusplus/core/lwf_data.cpp
    cplusplus/core/lwf_event.cpp
    cplusplus/core/lwf_eventbutton.cpp
    cplusplus/core/lwf_eventmovie.cpp
    cplusplus/core/lwf_format.cpp
    cplusplus/core/lwf_graphic.cpp
    cplusplus/core/lwf_input.cpp
    cplusplus/core/lwf_iobject.cpp
    cplusplus/core/lwf_lwfcontainer.cpp
    cplusplus/core/lwf_movie.cpp
    cplusplus/core/lwf_movieat.cpp
    cplusplus/core/lwf_movieop.cpp
    cplusplus/core/lwf_movieprop.cpp
    cplusplus/core/lwf_object.cpp
    cplusplus/core/lwf_particle.cpp
    cplusplus/core/lwf_programobj.cpp
    cplusplus/core/lwf_property.cpp
    cplusplus/core/lwf_text.cpp
    cplusplus/core/lwf_utility.cpp
    cplusplus/supports/lzma/Alloc.c
    cplusplus/supports/lzma/LzmaDec.c
)

if(WIN32)
  
elseif(APPLE)

else()

endif()

set(CMAKE_C_FLAGS_DEBUG "-g -Wall -DCOCOS2D_DEBUG=1")
set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(COCOS2D_ROOT  ${CMAKE_CURRENT_SOURCE_DIR}/../../cocos2d)
include_directories(
  cplusplus
  cplusplus/core
  cplusplus/cocos2dx
  cplusplus/supports/lzma
  ${COCOS2D_ROOT}
  ${COCOS2D_ROOT}/cocos
  ${COCOS2D_ROOT}/cocos/audio/include
  ${COCOS2D_ROOT}/cocos/2d
  ${COCOS2D_ROOT}/cocos/2d/renderer
  ${COCOS2D_ROOT}/cocos/2d/platform
  ${COCOS2D_ROOT}/cocos/2d/platform/desktop
  ${COCOS2D_ROOT}/cocos/2d/platform/linux
  ${COCOS2D_ROOT}/cocos/base
  ${COCOS2D_ROOT}/cocos/physics
  ${COCOS2D_ROOT}/cocos/editor-support
  ${COCOS2D_ROOT}/cocos/math/kazmath
)

add_library(lightwf STATIC
  ${LIGHTWF_SRC}
)


target_link_libraries(lightwf
 cocos2d
)

set_target_properties(lightwf
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

