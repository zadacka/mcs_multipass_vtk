#add_subdirectory(fbo)
#add_subdirectory(glsl)

# Add VTK, insist that it uses 6.2, 
# include vtkWrappingTools so that the path is correct and no linker errors
#find_package(VTK 6.2 EXACT REQUIRED COMPONENTS 
## core set from here: http://www.visitusers.org/index.php?title=VTK_6.0_Upgrade
##vtkFiltersFlowpaths # not available
#  vtkIOPLY            # not available
#  vtkCommonCore
#  vtkFiltersHybrid
#  vtkFiltersModeling
#  vtkInteractionStyle
#  vtkIOLegacy
#  vtkIOXML
#  vtkRenderingAnnotation
#  vtkRenderingFreeType
#  vtkRenderingFreeTypeOpenGL
#  vtkRenderingOpenGL
#  
#  NO_MODULE
#)

## set(VTK_DIR /usr/local/lib/vtk-5.10) (does nothing!)
find_package(VTK 5.10.1 EXACT REQUIRED NO_MODULE)

include(${VTK_USE_FILE})
# message(STATUS "VTK MAJOR VERSION " ${CMAKE_MAJOR_VERSION}) # reports V3: wrong!

find_package(GLEW REQUIRED)
#find_package(OpenGL)


set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS ${VTK_DEFINITIONS})

include_directories(
  ${VTK_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIR} 
#  fbo 
#  glsl 
  ${CMAKE_CURRENT_SOURCE_DIR}
)

####################
# Make multipass_vtk
####################
#set(EXE multipass_vtk)
#
# include_directories()
#
#set(SOURCES
#  multipass_vtk.cpp 
#  vtkSaliencyPass.cpp 
#  fbo/renderbuffer.cpp
#  fbo/framebufferObject.cpp
#  fbo/glErrorUtil.cpp
#  glsl/glinfo.cpp
#  glsl/glsl.cpp
#  glsl/glutils.cpp
#  vtkSaliencyPass.h
#  fbo/renderbuffer.h
#  fbo/framebufferObject.h
#  fbo/glErrorUtil.h
#  glsl/glinfo.h
#  glsl/glsl.h
#  glsl/glutils.h
#)
#
#add_executable(${EXE} ${SOURCES})
#
#target_link_libraries(${EXE} ${VTK_LIBRARIES} ${GLEW_LIBRARY} )
#
####################
# Make brain
####################
#
#set(EXE brain)
#set(SOURCES 
#  brain.cpp
#)
#add_executable(${EXE} ${SOURCES})
#target_link_libraries(${EXE} ${VTK_LIBRARIES} ${GLEW_LIBRARY} )
#
####################
# Make paraview_port
####################

set(EXE paraview_port)

set(SOURCES 
  paraview_port.cpp
  paraview_port/vtkOVRCamera.cxx    
  paraview_port/vtkOVRPostPass.h
  paraview_port/vtkDualStereoPass.cxx  
  paraview_port/vtkOVRCamera.h      
  paraview_port/vtkDualStereoPass.h    
  paraview_port/vtkOVRPostPass.cxx  
)
add_executable(${EXE} ${SOURCES})
target_link_libraries(${EXE} ${VTK_LIBRARIES} ${GLEW_LIBRARY} )

###################
###################
###################


#message(STATUS "OpenGL library: " ${OPENGL_LIBRARIES})
#message(STATUS "Glew library: " ${GLEW_LIBRARY})
#
#configure_file(../resources/Distortion.fs Distortion.fs COPYONLY)
#configure_file(../resources/Distortion.vs Distortion.vs COPYONLY)
#configure_file(../resources/simple.fs simple.fs COPYONLY)
#configure_file(../resources/simple.vs simple.vs COPYONLY)
#
configure_file(../resources/btain.vtk btain.vtk COPYONLY)

configure_file(../resources/vtkOVRPostShaderFull_fs.glsl 
  vtkOVRPostShaderFull_fs.glsl COPYONLY)
configure_file(../resources/vtkOVRPostShaderSimple_fs.glsl
  vtkOVRPostShaderSimple_fs.glsl COPYONLY)

