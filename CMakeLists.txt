cmake_minimum_required(VERSION 3.22.1)
project(RegisterCirclesLab VERSION 1.0 LANGUAGES CXX)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

add_executable(mali25RegisterCircles RegisterCircles.cxx)
target_link_libraries(mali25RegisterCircles ${ITK_LIBRARIES})
