cmake_minimum_required(VERSION 3.11)
project(byml CXX)

find_package(pybind11 REQUIRED)
pybind11_add_module(bymlplus
  main.cpp
)
add_library(byml::pybind11_binding ALIAS bymlplus)

target_compile_options(bymlplus PRIVATE -Wall -Wextra)
set_target_properties(bymlplus PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
)

find_package(range-v3 REQUIRED)
find_package(byml REQUIRED)
target_link_libraries(bymlplus PRIVATE range-v3 byml::byml byml::common)
