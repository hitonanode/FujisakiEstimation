include_directories( ${CMAKE_SOURCE_DIR}/hmm )
include_directories( ${CMAKE_SOURCE_DIR}/fujisaki )
include_directories( ${CMAKE_SOURCE_DIR}/utilities )
include_directories( ${CMAKE_SOURCE_DIR}/libs/json )


add_library(Emestimation STATIC
    em_estimation.cpp
    em_estimation.hpp
)
target_link_libraries(Emestimation Fujisaki)