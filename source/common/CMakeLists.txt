option(ENABLE_DEBUG_LOGGING "Enable debug logging" OFF)

add_library(common
  align.h
  binary_reader.h
  log.h
  swap.h
)
add_library(byml::common ALIAS common)
set_target_properties(common PROPERTIES LINKER_LANGUAGE CXX)

target_compile_options(common PRIVATE -Wall -Wextra)
set_target_properties(common PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
)

target_include_directories(common
PRIVATE
  ../
)

if(ENABLE_DEBUG_LOGGING)
  find_package(spdlog REQUIRED)
  target_link_libraries(common PRIVATE spdlog::spdlog)
  target_sources(common PRIVATE log.cpp)
  target_compile_definitions(common PUBLIC BYML_ENABLE_DEBUG_LOGGING)
endif()
