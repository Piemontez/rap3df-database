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

    #add_executable(freenect-cppview cppview.cpp)
    add_executable(freenect-facecolection main.cpp camera.cpp)

    #target_link_libraries(freenect-cppview freenect ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${CMAKE_THREAD_LIBS_INIT} ${MATH_LIB})
    target_link_libraries(freenect-facecolection freenect glfw ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${CMAKE_THREAD_LIBS_INIT} ${MATH_LIB})

    install(TARGETS freenect-facecolection
            DESTINATION bin)
  endif()
