#pragma once
#include <string>
#include <vector>
#include "json.hpp"

namespace stfmest
{
    struct InputData
    {
        double fs;
        std::vector<double> logf0;
        std::vector<double> vuv;
        std::vector<double> initial_up;
        std::vector<double> initial_ua;
        double initial_mub;
        // bool isAccentNumFixed = false;
        // bool isAdditionalBindingEnabled = false;
    };

    inline void to_json(nlohmann::json &j, const InputData &id_)
    {
        j = nlohmann::json{{"fs", id_.fs},
                           {"logf0", id_.logf0},
                           {"vuv", id_.vuv},
                           {"initial_up", id_.initial_up},
                           {"initial_ua", id_.initial_ua},
                           {"initial_mub", id_.initial_mub},
                        //    {"isAccentNumFixed", id_.isAccentNumFixed},
                        //    {"isAdditionalBindingEnabled", id_.isAdditionalBindingEnabled}
                           };
    }

    inline void from_json(const nlohmann::json &j, InputData &id_)
    {
        id_.fs = j.at("fs").get<double>();
        id_.logf0 = j.at("logf0").get<std::vector<double> >();
        id_.vuv = j.at("vuv").get<std::vector<double> >();
        id_.initial_up = j.at("initial_up").get<std::vector<double> >();
        id_.initial_ua = j.at("initial_ua").get<std::vector<double> >();
        id_.initial_mub = j.at("initial_mub").get<double>();
        if (j.count("isAccentNumFixed"))
        {
            // id_.isAccentNumFixed = j.at("isAccentNumFixed").get<bool>();
        }
        if (j.count("isAdditionalBindingEnabled"))
        {
            // id_.isAdditionalBindingEnabled = j.at("isAdditionalBindingEnabled").get<bool>();
        }
    }
}
