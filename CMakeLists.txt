cmake_minimum_required(VERSION 2.8.12.1)

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

find_package(OpenGL)
find_package(GLUT)


if (OPENGL_FOUND AND GLUT_FOUND)
  include_directories(. ${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR} ${freenect2_INCLUDE_DIR})

  add_executable(rap3df-viewer main_viewer.cpp)
  add_executable(rap3df-acquisition main.cpp context.cpp freenectdevice.cpp camera.cpp utils.cpp)

  target_link_libraries(rap3df-viewer freenect
      ${OPENGL_LIBRARIES}
      ${GLUT_LIBRARY})

  target_link_libraries(rap3df-acquisition

      #freenect
      ${freenect2_LIBRARIES}

      freeimage
      ${FreeImage_LIBRARIES}

      ${OPENGL_LIBRARIES}
      ${GLUT_LIBRARY}

      ${OpenCV_LIBRARIES})

  install(TARGETS rap3df-viewer DESTINATION bin)
  install(TARGETS rap3df-acquisition DESTINATION bin)
endif()
