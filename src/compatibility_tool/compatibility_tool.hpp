// 2018.03 Ryotaro Sato

#pragma once
#include <vector>
#include <string>
#include "cmdline.h"
#include "input_data.hpp"
#include "fujisaki.hpp"


namespace stfmest
{
    InputData rawDataFilesToInputData (cmdline::parser &args);


    std::vector<FujisakiCommand> rawVectorDoubleToFujisakiCommands (
        const std::vector<double> &mup, 
        const std::vector<double> &mua,
        double fs
        );

    std::vector<FujisakiCommand> _mux_to_fujisakicommands(
        const std::vector<double> &mux_,
        FilterType filtertype,
        double omega,
        double fs);    
}
