include_directories( ${CMAKE_SOURCE_DIR}/libs/json )
include_directories( ${CMAKE_SOURCE_DIR}/fujisaki )

add_library(Evaluation STATIC
    evaluation.cpp
    evaluation.hpp
)