#include "evaluation.hpp"

#include <algorithm>
#include <string>
#include <utility>

template<class Type_>
using vector = std::vector<Type_>;

#include <iostream>

namespace stfmest
{
    std::vector<std::pair<FilterType, CommandsCoincidenceResult> >
    evaluateByDP(const std::vector<FujisakiCommand> &reference_,
                const std::vector<FujisakiCommand> &estimated_,
                double S,
                double zeroThres)
    {
        std::vector<FujisakiCommand> reference = reference_;
        std::sort(reference.begin(), reference.end());
        std::vector<FujisakiCommand> estimated = estimated_;
        std::sort(estimated.begin(), estimated.end());
        vector<FilterType> dimToType;
        std::map<std::string, unsigned> typeToDim;
        for (auto com : reference)
        {
            if (std::find(dimToType.begin(), dimToType.end(), com.filtertype) == dimToType.end())
            {
                dimToType.push_back(com.filtertype);
                typeToDim[std::to_string(com.filtertype)] = dimToType.size() - 1;
            }
        }
        for (auto com : estimated)
        {
            if (std::find(dimToType.begin(), dimToType.end(), com.filtertype) == dimToType.end())
            {
                dimToType.push_back(com.filtertype);
                typeToDim[std::to_string(com.filtertype)] = dimToType.size() - 1;
            }
        }
        vector<std::pair<FilterType, CommandsCoincidenceResult> > result;
        for (auto commandtype : dimToType)
        {
            result.push_back(std::make_pair(commandtype, CommandsCoincidenceResult(S)));
        }

        unsigned referenceLen = reference.size();
        unsigned estimatedLen = estimated.size();
        vector<vector<vector<int> >  >isMatched(dimToType.size(), vector<vector<int> >(
                                                referenceLen, vector<int>(
                                                estimatedLen, 0)));

        for (unsigned iRef = 0; iRef<referenceLen; iRef++)
        {
            for (unsigned iEst=0; iEst<estimatedLen; iEst++)
            {
                isMatched[typeToDim[std::to_string(reference[iRef].filtertype)]][iRef][iEst]
                = coOccurCheck(reference[iRef], estimated[iEst], S, zeroThres);
            }
        }
        vector<vector<vector<int> > > numDP(dimToType.size(), vector<vector<int> >(
                                                referenceLen+1, vector<int>(
                                                estimatedLen+1, 0)));

        for (unsigned iType=0; iType<dimToType.size(); iType++)
        {
            for (unsigned iRef=0; iRef<referenceLen; iRef++)
            {
                for (unsigned iEst=0; iEst<estimatedLen; iEst++)
                {
                    numDP[iType][iRef+1][iEst+1] = std::max(numDP[iType][iRef][iEst] + isMatched[iType][iRef][iEst],
                                                            std::max(numDP[iType][iRef][iEst+1],
                                                                    numDP[iType][iRef+1][iEst]));
                }
            }

            int commNumInReference = 0;
            for (auto comm : reference) if (comm.filtertype == dimToType[iType]) ++commNumInReference;
            int commNumInEstimated = 0;
            for (auto comm : estimated) if (comm.filtertype == dimToType[iType]) ++commNumInEstimated;

            result[iType].second.totalNumInReference = commNumInReference;
            result[iType].second.totalNumInEstimated = commNumInEstimated;
            result[iType].second.matchedNum = numDP[iType][referenceLen][estimatedLen];
        }

        return result;
    }


    int coOccurCheck(const FujisakiCommand &com1, const FujisakiCommand &com2, double S, double zeroThres)
    {
        if (com1.filtertype != com2.filtertype) return 0;
        if (std::abs(com1.onset - com2.onset) + std::abs(com1.offset - com2.offset) - S * 2.0  > S * zeroThres)
        {
            return 0;
        }
        return 1;
    }
}
