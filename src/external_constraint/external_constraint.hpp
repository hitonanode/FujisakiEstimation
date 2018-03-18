// Imposing some path constraint
// by adding cost function to HMM output likelihood.
//
// 2018.03 Ryotaro Sato

#pragma once
#include <string>
#include <utility>
#include <vector>
#include "json.hpp"
#include "fujisaki.hpp"
#include "em_estimation.hpp"


namespace stfmest
{
    // Information of 2 GMMs constraining one accent command(onset/offset time distribution).
    struct StochasticCommandConstraint
    {
        double onBasetime, offBasetime;
        // GMM parameters
        std::vector<double> onWeights, offWeights;
        std::vector<double> onMeans, offMeans;
        std::vector<double> onSigmas, offSigmas;
    };

    inline void to_json(nlohmann::json &j, const StochasticCommandConstraint &scc)
    {
        j = nlohmann::json({{"onBasetime", scc.onBasetime},
                            {"offBasetime", scc.offBasetime},
                            {"onWeights", scc.onWeights},
                            {"offWeights", scc.offWeights},
                            {"onMeans", scc.onMeans},
                            {"offMeans", scc.offMeans},
                            {"onSigmas", scc.onSigmas},
                            {"offSigmas", scc.offSigmas}
                            });
    }

    inline void from_json(const nlohmann::json &j, StochasticCommandConstraint &scc)
    {
        scc.onBasetime = j.at("onBasetime").get<double>();
        scc.offBasetime = j.at("offBasetime").get<double>();
        scc.onWeights = j.at("onWeights").get<std::vector<double> >();
        scc.offWeights = j.at("offWeights").get<std::vector<double> >();
        scc.onMeans = j.at("onMeans").get<std::vector<double> >();
        scc.offMeans = j.at("offMeans").get<std::vector<double> >();
        scc.onSigmas = j.at("onSigmas").get<std::vector<double> >();
        scc.offSigmas = j.at("offSigmas").get<std::vector<double> >();
    }


    class EmEstimationConstrained : public EmEstimation
    {
        using EmEstimation::EmEstimation;
        bool flagConst = false;
    public:
        void imposeStochasticConst(const std::vector<StochasticCommandConstraint> &sccs);
        inline std::vector<std::vector<double> > getConstraintProb() { return constraintProbLog; }
    };
}
