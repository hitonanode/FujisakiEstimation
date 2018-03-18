// Treating general HMM topology
// consisting of hierarchical structure.
//
// 2018.03 Ryotaro Sato

#pragma once
#include <utility>
#include <vector>
#include "big_state.hpp"


namespace stfmest
{
    // Hmm: Contain topology information of HMM
    // consisting of hierarchical big/small states.
    class Hmm
    {
    private:
        std::vector<BigState> bigstates;
        unsigned num_bigstates; // No. of big states.
        std::vector<std::vector<std::pair<unsigned, double> > > transition_probs; // trans. prob. betw. big states.
        unsigned initial_bigstate; // Which big state at the initial frame
        unsigned final_bigstate; // Which big state at the last frame
    public:
        Hmm(): num_bigstates(0), initial_bigstate(0), final_bigstate(0)
        {
            _hmmInitialize();
        }
        
        
        // addState : append one big state
        //
        // Specify possible next big state No.'s by "connect"
        // (the first element in each pair represents the id of next big state,
        // followed by the second one representing transition probability.)
        bool addState(
            const BigState& bs,
            std::vector<std::pair<unsigned, double> > connect
        );
        
        
        bool setInitialState(unsigned i);
        bool setFinalState(unsigned i);
        std::string getStatus();

        inline unsigned getStateNum() { return num_bigstates; }
        inline unsigned getInitialState() { return initial_bigstate; }
        inline unsigned getFinalState() { return final_bigstate; }
        inline std::vector<BigState> getBigStates() { return bigstates; }
        inline std::vector<std::pair<unsigned, double> > getTransition(unsigned i_st)
        {
            return transition_probs[i_st];
        }
        inline BigState getBigState(unsigned bigstateNum) { return bigstates[bigstateNum]; }
        unsigned countByStateType(int keyAttribute); // count No. of big states which attribute matches.
    private:
        void _hmmInitialize();
    };
}
