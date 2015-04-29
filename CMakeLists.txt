
cmake_minimum_required(VERSION 2.8)

PROJECT(Skeleton)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

set(HEADER_FILES MyImage3D.h)
set(SOURCE_FILES MyImage3D.cpp main.cpp)
add_executable(Skeleton MACOSX_BUNDLE ${HEADER_FILES} ${SOURCE_FILES})

add_definitions(-std=c++11)

if(VTK_LIBRARIES)
  target_link_libraries(Skeleton ${VTK_LIBRARIES})
else()
  target_link_libraries(Skeleton vtkHybrid vtkWidgets)
endif()
