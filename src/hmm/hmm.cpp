#include "hmm.hpp"


namespace stfmest
{
    void Hmm::_hmmInitialize()
    {
        bigstates.clear();
        transition_probs.clear();
    }


    bool Hmm::addState(const BigState& bs, std::vector<std::pair<unsigned, double> > connect)
    {
        bigstates.push_back(bs);
        num_bigstates++;
        transition_probs.push_back(connect);
        return true;
    }


    bool Hmm::setInitialState(unsigned i)
    {
        initial_bigstate = i;
        return true;
    }


    bool Hmm::setFinalState(unsigned i)
    {
        final_bigstate = i;
        return true;
    }


    std::string Hmm::getStatus()
    {
        std::string str;
        str += "Big states: " + std::to_string(num_bigstates) + "\n";
        str += "Initial: " + std::to_string(initial_bigstate) + "\n";
        str += "Final: " + std::to_string(final_bigstate) + "\n";
        for (unsigned i=0; i<num_bigstates; i++)
        {
            str += std::to_string(i) + "<";
            for (auto p : transition_probs[i])
            {
                str += " " + std::to_string(p.first) + ":" + std::to_string(p.second);
            }
            str += " > " + bigstates[i].getStatus() + "\n";
        }
        return str;
    }

    unsigned Hmm::countByStateType(int keyAttribute)
    {
        unsigned countNum = 0;
        for (auto bigstate : bigstates)
        {
            if (bigstate.getAttribute() == keyAttribute) ++countNum;
        }
        return countNum;
    }
}
