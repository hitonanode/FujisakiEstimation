include_directories( ${CMAKE_SOURCE_DIR}/libs/json )

add_library(Hmm STATIC
    hmm.cpp
    hmm.hpp
    hmm_fujisaki.cpp
    hmm_fujisaki.hpp
    big_state.hpp
) 