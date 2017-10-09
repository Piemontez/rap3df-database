CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12.1)
project(RAP3DF)

if(WIN32 AND NOT MINGW)
  if(NOT DEFINED CMAKE_DEBUG_POSTFIX)
    set(CMAKE_DEBUG_POSTFIX "d")
  endif()
endif()

if(NOT DEFINED CMAKE_BUILD_TYPE)
  # No effect for multi-configuration generators (e.g. for Visual Studio)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose: RelWithDebInfo Release Debug MinSizeRel None")
endif()

#include_directories (${CMAKE_CURRENT_SOURCE_DIR}/include)

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
set(THREADS_USE_PTHREADS_WIN32 true)
find_package(Threads)
find_package(OpenGL)
find_package(GLUT)


if (Threads_FOUND AND OPENGL_FOUND AND GLUT_FOUND)
  include_directories(. ${THREADS_PTHREADS_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR} ${freenect2_INCLUDE_DIR})

  #add_executable(rap3df-viewer main_viewer.cpp utils.cpp)
  add_executable(rap3df-acquisition main.cpp context.cpp freenectdevice.cpp camera.cpp utils.cpp)

  #target_link_libraries(rap3df-viewer freenect ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${CMAKE_THREAD_LIBS_INIT} ${MATH_LIB})
  target_link_libraries(rap3df-acquisition freeimage freenect ${freenect2_LIBRARIES} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${CMAKE_THREAD_LIBS_INIT}
      ${MATH_LIB} ${GLFW3_LIBRARIES}
      ${FreeImage_LIBRARIES}
      ${OpenCV_LIBRARIES})

  #install(TARGETS rap3df-viewer DESTINATION bin)
  install(TARGETS rap3df-acquisition DESTINATION bin)
endif()














#CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12.1)

#if(WIN32 AND NOT MINGW)
#  if(NOT DEFINED CMAKE_DEBUG_POSTFIX)
#    set(CMAKE_DEBUG_POSTFIX "d")
#  endif()
#endif()

#IF(NOT DEFINED CMAKE_BUILD_TYPE)
#  # No effect for multi-configuration generators (e.g. for Visual Studio)
#  SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose: RelWithDebInfo Release Debug MinSizeRel None")
#ENDIF()

#PROJECT(libfreenect2_examples)

#SET(MY_DIR "/home/piemontez/Projects/KinectLibfreenect2")
#SET(DEPENDS_DIR "${MY_DIR}/../depends" CACHE STRING "Dependency directory")

#OPTION(ENABLE_OPENGL "Enable OpenGL support" ON)

## The example build system is standalone and will work out-of-tree with these files copied
#SET(freenect2_ROOT_DIR ${MY_DIR}/..)
#SET(flextGL_SOURCES ${freenect2_ROOT_DIR}/src/flextGL.cpp)
#SET(flextGL_INCLUDE_DIRS ${freenect2_ROOT_DIR}/src) # for flextGL.h

#FIND_PACKAGE(PkgConfig)    # try find PKGConfig as it will be used if found

## - Try to find GLFW3
##
## If no pkgconfig, define GLFW_ROOT to installation tree
## Will define the following:
## GLFW3_FOUND
## GLFW3_INCLUDE_DIRS
## GLFW3_LIBRARIES

#IF(PKG_CONFIG_FOUND)
#  IF(APPLE)
#    # homebrew or macports pkgconfig locations
#    SET(ENV{PKG_CONFIG_PATH} "/usr/local/opt/glfw3/lib/pkgconfig:/opt/local/lib/pkgconfig")
#  ENDIF()
#  SET(ENV{PKG_CONFIG_PATH} "${DEPENDS_DIR}/glfw/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
#  PKG_CHECK_MODULES(GLFW3 glfw3)

#  FIND_LIBRARY(GLFW3_LIBRARY
#    NAMES ${GLFW3_LIBRARIES}
#    HINTS ${GLFW3_LIBRARY_DIRS}
#  )
#  SET(GLFW3_LIBRARIES ${GLFW3_LIBRARY})

#ENDIF()

##FIND_PATH(GLFW3_INCLUDE_DIRS
##  GLFW/glfw3.h
##  DOC "GLFW include directory "
##  PATHS
##    "${DEPENDS_DIR}/glfw"
##    "$ENV{ProgramW6432}/glfw"
##    ENV GLFW_ROOT
##  PATH_SUFFIXES
##    include
##)

### directories in the official binary package
##IF(MINGW)
##  SET(_SUFFIX lib-mingw)
##ELSEIF(MSVC11)
##  SET(_SUFFIX lib-vc2012)
##ELSEIF(MSVC12)
##  SET(_SUFFIX lib-vc2013)
##ELSEIF(MSVC14)
##  SET(_SUFFIX lib-vc2015)
##ELSEIF(MSVC)
##  SET(_SUFFIX lib-vc2012)
##ENDIF()

##FIND_LIBRARY(GLFW3_LIBRARIES
##  NAMES glfw3dll glfw3
##  PATHS
##    "${DEPENDS_DIR}/glfw"
##    "$ENV{ProgramW6432}/glfw"
##    ENV GLFW_ROOT
##  PATH_SUFFIXES
##    lib
##    ${_SUFFIX}
##)

##IF(WIN32)
##FIND_FILE(GLFW3_DLL
##  glfw3.dll
##  PATHS
##    "${DEPENDS_DIR}/glfw"
##    "$ENV{ProgramW6432}/glfw"
##    ENV GLFW_ROOT
##  PATH_SUFFIXES
##    ${_SUFFIX}
##)
##ENDIF()

##INCLUDE(FindPackageHandleStandardArgs)
##FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLFW3 FOUND_VAR GLFW3_FOUND
##  REQUIRED_VARS GLFW3_LIBRARIES GLFW3_INCLUDE_DIRS)

##
##
##
##


#IF(TARGET freenect2)
#  MESSAGE(STATUS "Using in-tree freenect2 target")
#  SET(freenect2_LIBRARIES freenect2)
#  SET(freenect2_DLLS ${LIBFREENECT2_DLLS})
#ELSE()
#  FIND_PACKAGE(freenect2 REQUIRED)
#  # Out-of-tree build will have to have DLLs manually copied.
#ENDIF()

#INCLUDE_DIRECTORIES(
#  ${freenect2_INCLUDE_DIR}
#)

#SET(Protonect_src
#  Protonect.cpp
#)

#SET(Protonect_LIBRARIES
#  ${freenect2_LIBRARIES}
#)

#SET(Protonect_DLLS
#  ${freenect2_DLLS}
#)

##FIND_PACKAGE(GLFW3)
##FIND_PACKAGE(OpenGL)
#find_package(GLUT)

##INCLUDE_DIRECTORIES(
##  ${GLFW3_INCLUDE_DIRS}
##  ${flextGL_INCLUDE_DIRS}
##)

##LIST(APPEND Protonect_DLLS ${GLFW3_DLL})
##LIST(APPEND Protonect_src
##  viewer.cpp
##  ${flextGL_SOURCES}
##)
##LIST(APPEND Protonect_LIBRARIES
##  ${GLFW3_LIBRARIES}
##  ${OPENGL_gl_LIBRARY}
##  ${OPENGL_LIBRARIES}
##)
##ADD_DEFINITIONS(-DEXAMPLES_WITH_OPENGL_SUPPORT=1)


#find_package(GLUT)
#LIST(APPEND Protonect_LIBRARIES
#  ${GLUT_LIBRARY}
#)

#ADD_EXECUTABLE(rap3df-acquisition
#    main.cpp context.cpp freenectdevice.cpp camera.cpp utils.cpp
#)

#TARGET_LINK_LIBRARIES(rap3df-acquisition
#  freenect ${Protonect_LIBRARIES}
#)

#IF(WIN32)
#  INSTALL(TARGETS rap3df-acquisition DESTINATION bin)
#  LIST(REMOVE_DUPLICATES Protonect_DLLS)
#  FOREACH(FILEI ${Protonect_DLLS})
#    ADD_CUSTOM_COMMAND(TARGET Protonect POST_BUILD
#      COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FILEI} $<TARGET_FILE_DIR:Protonect>
#    )
#  ENDFOREACH(FILEI)
#  INSTALL(FILES ${Protonect_DLLS} DESTINATION bin)
#ENDIF()
