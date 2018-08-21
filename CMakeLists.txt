cmake_minimum_required(VERSION 2.8.12.1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

project(RAP3DF)

if(WIN32 AND NOT MINGW)
  if(NOT DEFINED CMAKE_DEBUG_POSTFIX)
    set(CMAKE_DEBUG_POSTFIX "d")
  endif()
endif()


#find freenect2
if(TARGET freenect2)
  message(STATUS "Using in-tree freenect2 target")
  set(freenect2_LIBRARIES freenect2)
  set(freenect2_DLLS ${LIBFREENECT2_DLLS})
else()
  find_package(freenect2 REQUIRED)
  # Out-of-tree build will have to have DLLs manually copied.
endif()

#other libs dependences

set(PLATFORM_SPECIFIC_LIBS "-lpthread")
set(OpenGL_GL_PREFERENCE "LEGACY")

find_package(OpenGL)
find_package(GLUT)
#find_package(jsoncpp REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)

if (OPENGL_FOUND AND GLUT_FOUND)
  include_directories(. ${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR} ${freenect2_INCLUDE_DIR})

#  add_executable(rap3df-viewer main_viewer.cpp)
  add_executable(rap3df-acquisition main.cpp context.cpp actions.cpp freenectdevice.cpp camera.cpp utils.cpp viewport.cpp)

#  target_link_libraries(rap3df-viewer
#
#      #freenect
#
#      ${OPENGL_LIBRARIES}
#      ${GLUT_LIBRARY}
#      ${JSONCPP_LIBRARIES})

  target_link_libraries(rap3df-acquisition
      #freenect
      ${freenect2_LIBRARIES}

      freeimage
      ${FreeImage_LIBRARIES}

      ${OPENGL_LIBRARIES}
      ${GLUT_LIBRARY}
      ${JSONCPP_LIBRARIES})

  #install(TARGETS rap3df-viewer DESTINATION bin)
  install(TARGETS rap3df-acquisition DESTINATION bin)
endif()
