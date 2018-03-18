// Evaluation for estimation results by "Deletion/Insertion rate".
//
// 2018.03 Ryotaro Sato

#pragma once

#include "fujisaki.hpp"

#include <utility>
#include <vector>
#include "json.hpp"


namespace stfmest
{
    struct CommandsCoincidenceResult
    {
        int totalNumInReference = 0;
        int totalNumInEstimated = 0;
        int matchedNum = 0;
        double allowedTimeLag = 0.0;
        CommandsCoincidenceResult() {};
        CommandsCoincidenceResult(double S): allowedTimeLag(S) {};
    };


    inline void to_json(nlohmann::json &j, const CommandsCoincidenceResult &ccr)
    {
        j = nlohmann::json{{"totalNumInReference", ccr.totalNumInReference},
                        {"totalNumInEstimated", ccr.totalNumInEstimated},
                        {"matchedNum", ccr.matchedNum},
                        {"allowedTimeLag", ccr.allowedTimeLag}
                        };
    }


    inline void from_json(const nlohmann::json &j, CommandsCoincidenceResult &ccr)
    {
        ccr.totalNumInReference = j.at("totalNumInReference").get<int>();
        ccr.totalNumInEstimated = j.at("totalNumInEstimated").get<int>();
        ccr.matchedNum = j.at("matchedNum").get<int>();
        ccr.allowedTimeLag = j.at("allowedTimeLag").get<double>();
    }


    // Main function for evaluation
    std::vector<std::pair<FilterType, CommandsCoincidenceResult> > evaluateByDP
    (
        const std::vector<FujisakiCommand> &reference,
        const std::vector<FujisakiCommand> &estimated,
        double S, // Threshold time difference.
        double zeroThres // Set very-small nonzero to avoid numerical instability of results.
    );


    int coOccurCheck(const FujisakiCommand &com1, const FujisakiCommand &com2, double S, double zeroThres);
}
