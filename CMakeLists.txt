# Pretty much everyone is going to need the main includes
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/include)

#install(FILES libfreenect.hpp
#        DESTINATION ${PROJECT_INCLUDE_INSTALL_DIR})

  set(THREADS_USE_PTHREADS_WIN32 true)
  find_package(Threads)
  find_package(OpenGL)
  find_package(GLUT)

  if (Threads_FOUND AND OPENGL_FOUND AND GLUT_FOUND)
    include_directories(. ${THREADS_PTHREADS_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR})

    add_executable(rap3df-viewer main_viewer.cpp utils.cpp)
    add_executable(rap3df-acquisition main.cpp context.cpp freenectdevice.cpp camera.cpp utils.cpp)

    target_link_libraries(rap3df-viewer freenect ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${CMAKE_THREAD_LIBS_INIT} ${MATH_LIB})
    target_link_libraries(rap3df-acquisition freenect glfw ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${CMAKE_THREAD_LIBS_INIT} ${MATH_LIB})

    install(TARGETS rap3df-viewer DESTINATION bin)
    install(TARGETS rap3df-acquisition DESTINATION bin)
  endif()
