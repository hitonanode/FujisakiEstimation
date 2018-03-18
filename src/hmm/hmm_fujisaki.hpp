// The modules treating the HMM structure
// especially related to the Fujisaki command estimation
//
// 2018.03, Ryotaro Sato

#pragma once

#include <vector>
#include "hmm.hpp"
#include "json.hpp"


namespace stfmest
{
    // HMM State Classification
    enum BigStateType
    {
        STATE_BEGIN = 0,
        STATE_END = 1,
        STATE_RESET0 = 10,
        STATE_RESET1 = 11,
        STATE_RESET_OTHER = 12,
        STATE_PHRASE = 20,
        STATE_ACCENT = 30
    };


    // The struct containing all information about the
    // transition probability of HMM between big states
    // and duration distribution in each big states.
    struct TransParams
    {
        std::vector<double> r0duration;
        std::vector<double> r1duration;
        std::vector<double> acduration;
        std::vector<double> phduration;
        double prob_ator0, prob_ator1;

        // A method is used just like regularization
        // to avoid such a situation there's no solution
        void regularize(double distributionFactor);
    };


    void to_json(nlohmann::json &j, const TransParams &tp);
    void from_json(const nlohmann::json &j, TransParams &tp);


    // Make Loop HMM to estimate commands from F0
    // (implementation of [Kameoka+, 2015])
    Hmm makeLoopFujisakiHmm(
        unsigned phraseBranchNum, // The num. of phrase-on big states.
        unsigned accentBranchNum, // The num. of accent-on big states.
        const TransParams &st,    // Information of hmm transition probs.
        unsigned margin_length    // The possible maximum length of no-command section in the beginnig/ending.
    );


    // Make Serialized HMM to estimate commands from F0
    // in such a situation that num. of accent commands is fixed.
    Hmm makeSerializedFujisakiHmm(
        unsigned accent_num, // The num. of accent commands in the input speech.
        const TransParams &st,
        unsigned margin_length, // The possible maximum length of no-command section in the beginnig/ending.
        unsigned longestAccentCommFrame, // The possible maximum length of estimated accent commands.
        unsigned phraseBranchNum, // The num. of phrase-on big states in a cluster.
        unsigned accentBranchNum // The num. of accent-on big states in a cluster.
    );
}
