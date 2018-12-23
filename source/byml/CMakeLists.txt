cmake_minimum_required(VERSION 3.11)
project(byml CXX)

add_library(byml
  ../../include/byml/binary_format.h
  ../../include/byml/byml.h
  ../../include/byml/types.h
  ../../include/byml/value.h
  byml.cpp
  container_util.h
  value.cpp
)
add_library(byml::byml ALIAS byml)

target_compile_options(byml PRIVATE -Wall -Wextra)
set_target_properties(byml PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
)

target_include_directories(byml
PUBLIC ../../include
PRIVATE
  ../
)

find_package(range-v3 REQUIRED)
target_link_libraries(byml
INTERFACE
  range-v3
PRIVATE
  byml::common
)
