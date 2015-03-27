
cmake_minimum_required(VERSION 2.8)

PROJECT(Skeleton)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

add_executable(Skeleton MACOSX_BUNDLE main.cpp)

if(VTK_LIBRARIES)
  target_link_libraries(Skeleton ${VTK_LIBRARIES})
else()
  target_link_libraries(Skeleton vtkHybrid vtkWidgets)
endif()
