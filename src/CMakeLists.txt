#add_subdirectory(fbo)
#add_subdirectory(glsl)

# CMake modules are in src just now...
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})

# Add VTK, insist that it uses 6.2, 
# include vtkWrappingTools so that the path is correct
find_package(VTK 6.2 EXACT REQUIRED COMPONENTS vtkRenderingOpenGL vtkInteractionStyle NO_MODULE)
# message(STATUS "VTK MAJOR VERSION " ${CMAKE_MAJOR_VERSION}) # reports V3: wrong!

include_directories(${VTK_INCLUDE_DIRS} )
include(${VTK_USE_FILE})
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS ${VTK_DEFINITIONS})

find_package(GLEW REQUIRED)


include_directories(${GLEW_INCLUDE_DIR} fbo glsl ${CMAKE_CURRENT_SOURCE_DIR})

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
message(STATUS "Glew library: " ${GLEW_LIBRARY})


