#include "external_constraint.hpp"

#include <iostream>
const double M_PI = 3.14159265358979323846;


namespace stfmest
{
    std::vector<double> _tmp_calcGmmLogsumexp (
        double basetime,
        const std::vector<double> &weights,
        const std::vector<double> &means,
        const std::vector<double> &sigmas,
        int frameNum,
        double fs
        )
    {

        std::vector<double> result_logprob(frameNum);

        for (unsigned iFr=0; iFr<frameNum; iFr++)
        {
            double t_diff = (double)iFr / fs - basetime;
            std::vector<double> powers(weights.size());
            for (unsigned i=0; i<powers.size(); i++)
            {
                powers[i] = - 0.5 * (t_diff - means[i]) * (t_diff - means[i]) / sigmas[i] / sigmas[i];
                // std::cout << powers[i] << std::endl;
            }
            auto argmaxpower = std::max_element(powers.begin(), powers.end());

            double prob_tmp = 0.0;
            for (unsigned i=0; i<powers.size(); i++)
            {
                prob_tmp += weights[i] / sqrt(2.0 * M_PI) / sigmas[i] * exp(powers[i] - *argmaxpower);
            }
            result_logprob[iFr] = log(prob_tmp) + *argmaxpower;
        }
        return result_logprob;
    }

    void EmEstimationConstrained::imposeStochasticConst(const std::vector<StochasticCommandConstraint> &sccs)
    {
        // std::cout << "Start stochastic const." << std::endl;
        flagConst = true;
        constraintProbLog = std::vector<std::vector<double> >(frameNum, std::vector<double>(stateNum, 0.0));
        // return;
        // std::cout << hmm.getStatus() << std::endl;

        // std::cout << "sccs length:" << sccs.size() << std::endl;
        for (unsigned iAcc=0; iAcc<sccs.size(); iAcc++)
        {
            std::vector<double> onsetDist = _tmp_calcGmmLogsumexp (
                sccs[iAcc].onBasetime, sccs[iAcc].onWeights,
                sccs[iAcc].onMeans, sccs[iAcc].onSigmas,
                frameNum, input.fs
                );
            auto onsetDistMaxIter = std::max(onsetDist.begin(), onsetDist.end());

            std::vector<double> offsetDist = _tmp_calcGmmLogsumexp (
                sccs[iAcc].offBasetime, sccs[iAcc].offWeights,
                sccs[iAcc].offMeans, sccs[iAcc].offSigmas,
                frameNum, input.fs
                );
            auto offsetDistMaxIter = std::max(offsetDist.begin(), offsetDist.end());
            
            for (unsigned iBranch=0; iBranch<config.accentBranchNum; iBranch++)
            {
                unsigned iSsBegin = stateNum;
                unsigned iSsEnd = stateNum;
                for (unsigned iSs=0; iSs<stateNum; iSs++)
                {
                    if (iSsBegin == stateNum && smallStates[iSs].bigstateId == accentNumToBigState[iAcc*config.accentBranchNum+iBranch]) iSsBegin = iSs;
                    if (iSsBegin < stateNum && iSsEnd == stateNum && smallStates[iSs].bigstateId != accentNumToBigState[iAcc*config.accentBranchNum+iBranch]) iSsEnd = iSs-1;
                }
                // std::cout << iBranch << " " << iSsBegin << " " << iSsEnd << std::endl;

                for (unsigned prevSsId : smallStates[iSsBegin].backwardConnects)
                {
                    // std::cout << iSsBegin << " " << prevSsId << std::endl;
                    for (unsigned iFr=1; iFr<frameNum; iFr++)
                    {
                        constraintProbLog[iFr-1][prevSsId] = onsetDist[iFr];
                        if (onsetDist[iFr] < *onsetDistMaxIter - 4.6 )
                        {
                            isReachable[iFr-1][prevSsId] = false;
                            // std::cout << iFr << " " << prevSsId << std::endl;
                        }
                    }
                }
                for (unsigned iFr=0; iFr<frameNum-1; iFr++)
                {
                    constraintProbLog[iFr+1][iSsEnd] = offsetDist[iFr];
                    if (offsetDist[iFr] < *offsetDistMaxIter-4.6) isReachable[iFr+1][iSsEnd] = false;
                }
            }
        }
        _updateReachableStateInfo();
    }
}
