// Defining "big states" in the HMM
// which is defined as serially connected
// small states.
//
// 2018.03 Ryotaro Sato

#pragma once
#include <string>
#include <vector>


namespace stfmest
{
    // BigState: HMM big state
    class BigState
    {
    public:
        BigState(const std::vector<double> &duration, const int &attribute_):
            dist_duration(duration),
            num_smallstates(duration.size()),
            attribute(attribute_)
        { ; }
        inline int getAttribute() { return attribute; }
        inline void setAttribute(const int &attribute_) { attribute = attribute_; }

        inline unsigned getSmallStateNum() { return num_smallstates; }
        inline std::vector<double> getDurationDist() { return dist_duration; }

        inline std::string getStatus()
        {
            std::string retval = "Attribute: " + std::to_string(attribute)
                            + ", Length: " + std::to_string(num_smallstates);
            return retval;
        }
    private:
        // dist_duration: Duration distribution of big state.
        // (dist_duration[i] == Prob(Duration == i+1 frames))
        std::vector<double> dist_duration;
        unsigned num_smallstates; // num of contained small states.
        int attribute; // Characterize each big state.
    };
}
