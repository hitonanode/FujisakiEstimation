#pragma once
#include <vector>
#include "json.hpp"


namespace stfmest
{
    struct EstimationResult
    {
        std::vector<double> mup;
        std::vector<double> mua;
        double mub;
        std::vector<double> Cp;
        std::vector<double> Ca;
        std::vector<int> bigs;

        std::vector<FujisakiCommand> commands;
        std::vector<double> regeneratedlf0;
        double rmse;
        int voicedFrameNum;
    };


    inline void to_json(nlohmann::json &j, const EstimationResult &er)
    {
        j = nlohmann::json{{"mup", er.mup}, {"mua", er.mua}, {"mub", er.mub},
                        {"Cp", er.Cp}, {"Ca", er.Ca}, {"bigs", er.bigs},
                        {"commands", er.commands}, {"regeneratedlf0", er.regeneratedlf0},
                        {"rmse", er.rmse}, {"voicedFrameNum", er.voicedFrameNum}};
    }
}