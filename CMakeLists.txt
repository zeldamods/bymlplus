cmake_minimum_required(VERSION 3.11)
project(byml CXX)

if(CMAKE_GENERATOR MATCHES "Ninja")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color")
endif()

add_subdirectory(source/common)
add_subdirectory(source/byml)

set_target_properties(common byml
PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

export(TARGETS common byml NAMESPACE byml:: FILE bymlConfig.cmake)
export(PACKAGE byml)
