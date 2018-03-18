include_directories( ${CMAKE_SOURCE_DIR}/libs/cmdline )
include_directories( ${CMAKE_SOURCE_DIR}/libs/json )
include_directories( ${CMAKE_SOURCE_DIR}/utilities )
include_directories( ${CMAKE_SOURCE_DIR}/hmm )
include_directories( ${CMAKE_SOURCE_DIR}/fujisaki )
include_directories( ${CMAKE_SOURCE_DIR}/emestimation )
include_directories( ${CMAKE_SOURCE_DIR}/evaluation )
include_directories( ${CMAKE_SOURCE_DIR}/external_constraint )

add_executable(StatisticalFujisakiEst main.cpp)
target_link_libraries(StatisticalFujisakiEst Utility Hmm Fujisaki Emestimation
                      ExternalConstraint Evaluation)
