include_directories( ${CMAKE_SOURCE_DIR}/hmm )
include_directories( ${CMAKE_SOURCE_DIR}/fujisaki )
include_directories( ${CMAKE_SOURCE_DIR}/emestimation )
include_directories( ${CMAKE_SOURCE_DIR}/utilities )
include_directories( ${CMAKE_SOURCE_DIR}/libs/json )


add_library(ExternalConstraint STATIC
    external_constraint.cpp
    external_constraint.hpp
)
target_link_libraries(ExternalConstraint Fujisaki Hmm Emestimation)