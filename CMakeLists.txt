
cmake_minimum_required(VERSION 2.8)

PROJECT(Cone)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

add_executable(Cone MACOSX_BUNDLE Cone)

if(VTK_LIBRARIES)
  target_link_libraries(Cone ${VTK_LIBRARIES})
else()
  target_link_libraries(Cone vtkHybrid vtkWidgets)
endif()
