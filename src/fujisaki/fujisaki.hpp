// The Fujisaki Model
//
// 2018.03 Ryotaro Sato

#pragma once

#include <cmath>
#include <vector>
#include "json.hpp"


namespace stfmest
{
    enum FilterType
    {
        CMD_PHRASE,
        CMD_ACCENT
    };


    // impulse response
    inline double impulse_response(double omega, double time_)
    {
        return time_>0.0 ? omega * omega * time_ * exp(-omega * time_) : 0.0;
    }


    // step response
    inline double step_response(double omega, double time_)
    {
        return time_>0.0 ? 1.0 - (1.0 + omega * time_) * exp(-omega * time_) : 0.0;
    }


    struct FujisakiCommand
    {
        FilterType filtertype;
        double onset;
        double offset;
        double integratedAmplitude; // To avoid confusion of notation
        double omega; // Angular freq. of the filter
        FujisakiCommand() {}
        FujisakiCommand(FilterType filterType, double onset, double offset, 
                        double integratedAmplitude, double omega):
                        filtertype(filterType), onset(onset), offset(offset), integratedAmplitude(integratedAmplitude), omega(omega) {}
    };

    inline bool operator<(const FujisakiCommand& left, const FujisakiCommand& right)
    {
        return left.onset < right.onset;
    }

    inline bool operator>(const FujisakiCommand& left, const FujisakiCommand& right)
    {
        return left.onset > right.onset;
    }

    inline void to_json(nlohmann::json &j, const FujisakiCommand &fc)
    {
        j = nlohmann::json{{"FilterType", (int)fc.filtertype},
                        {"onset", fc.onset},
                        {"offset", fc.offset},
                        {"integratedAmplitude", fc.integratedAmplitude},
                        {"omega", fc.omega}};
    }


    inline void from_json(const nlohmann::json &j, FujisakiCommand &fc)
    {
        fc.filtertype = static_cast<FilterType>(j.at("FilterType").get<int>());
        fc.onset = j.at("onset").get<double>();
        fc.offset = j.at("offset").get<double>();
        fc.integratedAmplitude = j.at("integratedAmplitude").get<double>();
        fc.omega = j.at("omega").get<double>();
    }


    std::vector<double> criticalfilter(const FujisakiCommand &command, double fs, int frameNum);
    std::vector<double> criticalfilter(const std::vector<FujisakiCommand> &commands, double mub, double fs, int frameNum);


    double rmse(const std::vector<double> &lf0a, const std::vector<double> &lf0b, const std::vector<double> &vuv, double vuvThres);
}
