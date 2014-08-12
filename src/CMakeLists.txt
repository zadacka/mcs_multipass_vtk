#add_subdirectory(fbo)
#add_subdirectory(glsl)

# Add VTK, insist that it uses 6.2, 
# include vtkWrappingTools so that the path is correct
#find_package(VTK 6.2 EXACT REQUIRED COMPONENTS vtkRenderingOpenGL vtkInteractionStyle NO_MODULE)
set(VTK_DIR /usr/local/lib/vtk-5.10)
find_package(VTK 5.10.1 EXACT REQUIRED)
# message(STATUS "VTK MAJOR VERSION " ${CMAKE_MAJOR_VERSION}) # reports V3: wrong!

find_package(GLEW REQUIRED)
#find_package(OpenGL)

include(${VTK_USE_FILE})
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS ${VTK_DEFINITIONS})

include_directories(
  ${VTK_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIR} 
  fbo 
  glsl 
  ${CMAKE_CURRENT_SOURCE_DIR}
#  ${OPENGL_INCLUDE_DIR}
)

####################
# Make multipass_vtk
####################
set(EXE multipass_vtk)

# include_directories()

set(SOURCES
  multipass_vtk.cpp 
  vtkSaliencyPass.cpp 
  fbo/renderbuffer.cpp
  fbo/framebufferObject.cpp
  fbo/glErrorUtil.cpp
  glsl/glinfo.cpp
  glsl/glsl.cpp
  glsl/glutils.cpp
  vtkSaliencyPass.h
  fbo/renderbuffer.h
  fbo/framebufferObject.h
  fbo/glErrorUtil.h
  glsl/glinfo.h
  glsl/glsl.h
  glsl/glutils.h
)

add_executable(${EXE} ${SOURCES})

target_link_libraries(${EXE} ${VTK_LIBRARIES} ${GLEW_LIBRARY} )

####################
# Make multipass_vtk
####################

set(EXE brain)
set(SOURCES 
  brain.cpp
)
add_executable(${EXE} ${SOURCES})
target_link_libraries(${EXE} ${VTK_LIBRARIES} ${GLEW_LIBRARY} )



message(STATUS "OpenGL library: " ${OPENGL_LIBRARIES})
message(STATUS "Glew library: " ${GLEW_LIBRARY})

configure_file(../resources/Distortion.fs Distortion.fs COPYONLY)
configure_file(../resources/Distortion.vs Distortion.vs COPYONLY)
configure_file(../resources/simple.fs simple.fs COPYONLY)
configure_file(../resources/simple.vs simple.vs COPYONLY)

