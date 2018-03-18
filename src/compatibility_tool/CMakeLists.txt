include_directories( ${CMAKE_SOURCE_DIR}/utilities )
include_directories( ${CMAKE_SOURCE_DIR}/fujisaki )
include_directories( ${CMAKE_SOURCE_DIR}/libs/json )
include_directories( ${CMAKE_SOURCE_DIR}/libs/cmdline )

add_executable(CompatibilityTool
    compatibility_tool.cpp
    compatibility_tool.hpp
    )

target_link_libraries(CompatibilityTool Utility)
