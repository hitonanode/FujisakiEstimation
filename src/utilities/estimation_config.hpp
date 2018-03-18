#pragma once
#include "json.hpp"


namespace stfmest
{
    struct EstimationConfig
    {
        // HMM global structure
        bool isHmmSerialized = false;
        int phraseBigStateNum; // (Optional)Effective iff isHmmSerialized == false.
        int accentBigStateNum;
        int phraseBranchNum = 1;
        int accentBranchNum = 1;

        // HMM detail preferences
        int phraseOnDuration;
        bool enableLimitedDurationExtension = true; // if false, always extend the duration prob.
        double durationExtensionFactor = 0.0;

        // EM algorithm preferences
        bool isHardEmEnabled = true;
        int iterationNum;
        int mstepUpdateNumPerIteration;
        int perturbSearchWidth;

        // Model parameters
        double defaultAlpha = 3.0;
        double defaultBeta = 20.0;
        double defaultSigmap2;
        double defaultSigmaa2;
        double defaultSigman2_voiced;
        double defaultSigman2_unvoiced;

        // constants for numerical calculation
        double regularizerOffset;
        double zeroThreshold;
        double inf;
    };

    inline void to_json(nlohmann::json& j, const EstimationConfig& ec)
    {
        j = nlohmann::json{{"isHmmSerialized", ec.isHmmSerialized},
                           {"accentBigStateNum", ec.accentBigStateNum},
                           {"phraseOnDuration", ec.phraseOnDuration},
                           {"enableLimitedDurationExtension", ec.enableLimitedDurationExtension},
                           {"durationExtensionFactor", ec.durationExtensionFactor},
                           {"isHardEmEnabled", ec.isHardEmEnabled},
                           {"iterationNum", ec.iterationNum},
                           {"mstepUpdateNumPerIteration", ec.mstepUpdateNumPerIteration},
                           {"perturbSearchWidth", ec.perturbSearchWidth},
                           {"defaultAlpha", ec.defaultAlpha},
                           {"defaultBeta", ec.defaultBeta},
                           {"defaultSigmap2", ec.defaultSigmap2},
                           {"defaultSigmaa2", ec.defaultSigmaa2},
                           {"defaultSigman2_voiced", ec.defaultSigman2_voiced},
                           {"defaultSigman2_unvoiced", ec.defaultSigman2_unvoiced},
                           {"regularizerOffset", ec.regularizerOffset},
                           {"zeroThreshold", ec.zeroThreshold},
                           {"inf", ec.inf}
                           };
        if (!ec.isHmmSerialized)
        {
            j["phraseBigStateNum"] = ec.phraseBigStateNum;
        }
    }

    inline void from_json(const nlohmann::json& j, EstimationConfig& ec)
    {
        ec.isHmmSerialized = j.at("isHmmSerialized").get<bool>();
        
        ec.accentBigStateNum = j.at("accentBigStateNum").get<int>();
        ec.phraseOnDuration = j.at("phraseOnDuration").get<int>();
        ec.isHardEmEnabled = j.at("isHardEmEnabled").get<bool>();
        ec.iterationNum = j.at("iterationNum").get<int>();
        ec.mstepUpdateNumPerIteration = j.at("mstepUpdateNumPerIteration").get<int>();
        ec.perturbSearchWidth = j.at("perturbSearchWidth").get<int>();
        ec.defaultAlpha = j.at("defaultAlpha").get<double>();
        ec.defaultBeta = j.at("defaultBeta").get<double>();
        ec.defaultSigmap2 = j.at("defaultSigmap2").get<double>();
        ec.defaultSigmaa2 = j.at("defaultSigmaa2").get<double>();
        ec.defaultSigman2_voiced = j.at("defaultSigman2_voiced").get<double>();
        ec.defaultSigman2_unvoiced = j.at("defaultSigman2_unvoiced").get<double>();
        ec.regularizerOffset = j.at("regularizerOffset").get<double>();
        ec.zeroThreshold = j.at("zeroThreshold").get<double>();
        ec.inf = j.at("inf").get<double>();

        if (!ec.isHmmSerialized)
        {
            ec.phraseBigStateNum = j.at("phraseBigStateNum").get<int>();
        }

        if (j.count("enableLimitedDurationExtension"))
        {
            ec.enableLimitedDurationExtension = j.at("enableLimitedDurationExtension").get<bool>();
        }
        if (j.count("durationExtensionFactor"))
        {
            ec.durationExtensionFactor = j.at("durationExtensionFactor").get<double>();
        }
    }
}