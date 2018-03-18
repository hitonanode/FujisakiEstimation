include_directories( ${CMAKE_SOURCE_DIR}/libs/json )
add_library(Utility STATIC
    error_codes.hpp
    estimation_config.hpp
    estimation_result.hpp
    input_data.hpp
    iofile.cpp
    iofile.hpp
    small_state.hpp
    timer.hpp
) 