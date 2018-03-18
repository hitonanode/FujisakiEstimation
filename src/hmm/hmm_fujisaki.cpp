#include "hmm_fujisaki.hpp"
#include <iostream>

using pair = std::pair<unsigned, double>;
template<class Type_>
using vector = std::vector<Type_>;


namespace stfmest
{
    void to_json(nlohmann::json &j, const TransParams &tp)
    {
        j = nlohmann::json{{"Duration_r0", tp.r0duration},
                           {"Duration_r1", tp.r1duration},
                           {"Duration_phrase", tp.phduration},
                           {"Duration_accent", tp.acduration},
                           {"Prob_accentTor0", tp.prob_ator0},
                           {"Prob_accentTor1", tp.prob_ator1}
                           };
    }

    void from_json(const nlohmann::json &j, TransParams &tp)
    {
        tp.r0duration = j.at("Duration_r0").get<vector<double> >();
        tp.r1duration = j.at("Duration_r1").get<vector<double> >();
        tp.phduration = j.at("Duration_phrase").get<vector<double> >();
        tp.acduration = j.at("Duration_accent").get<vector<double> >();
        tp.prob_ator0 = j.at("Prob_accentTor0").get<double>();
        tp.prob_ator1 = j.at("Prob_accentTor1").get<double>();
    }

    void TransParams::regularize(double distributionFactor)
    {
        unsigned accentLenDefault = acduration.size();
        unsigned whereNonzeroFirst = accentLenDefault;
        for (unsigned i=0; i<accentLenDefault; i++)
        {
            if (acduration[i] > 0.0)
            {
                whereNonzeroFirst = i;
                break;
            }
        }
        if (whereNonzeroFirst == accentLenDefault) return;


        unsigned countZero = accentLenDefault;
        double probSum = 0.0;
        for (unsigned i=whereNonzeroFirst; i<accentLenDefault; i++)
        {
            probSum += acduration[i];
            if (acduration[i] == 0.0) ++countZero;
        }
        double regularizer = probSum * distributionFactor / (double)countZero;
        
        for (unsigned i=whereNonzeroFirst; i<accentLenDefault; i++)
        {
            if (acduration[i] == 0.0)
            {
                acduration[i] = regularizer;
            }
            else
            {
                acduration[i] *= (1.0 - distributionFactor);
            }
        }
        std::vector<double> tmp(accentLenDefault, regularizer);
        acduration.insert(acduration.end(), tmp.begin(), tmp.end());
    }


    Hmm makeSerializedFujisakiHmm(unsigned accent_num, const TransParams &st, unsigned margin_length,
    unsigned longestAccentCommFrame, unsigned phraseBranchNum, unsigned accentBranchNum)
    {
        Hmm hmm;

        vector<double> prob_rinf(margin_length, 1.0);

        BigState bs_rinf(prob_rinf, static_cast<int>(STATE_RESET_OTHER));
        BigState bs_r0(st.r0duration, static_cast<int>(STATE_RESET0));
        BigState bs_r1(st.r1duration, static_cast<int>(STATE_RESET1));

        BigState bs_ac(vector<double>(longestAccentCommFrame, 1.0), static_cast<int>(STATE_ACCENT));
        BigState bs_ph(st.phduration, static_cast<int>(STATE_PHRASE));
        BigState bs_begin(vector<double>(1, 1.0), static_cast<int>(STATE_BEGIN));
        BigState bs_end(vector<double>(1, 1.0), static_cast<int>(STATE_END));

        hmm.addState(bs_begin, vector<pair>{pair(1, 1.0)});

        vector<pair> to_phrase;
        for (unsigned i_ph=0; i_ph<phraseBranchNum; i_ph++)
        {
            to_phrase.push_back(pair(2 + i_ph, 1.0/* / (double)phraseBranchNum */));
        }
        hmm.addState(bs_rinf, to_phrase);
        
        for (unsigned i_ph=0; i_ph<phraseBranchNum; i_ph++)
        {
            hmm.addState(bs_ph, vector<pair>{pair(2+phraseBranchNum, 1.0)});
        }

        double ator0 = st.prob_ator0 / (st.prob_ator0 + st.prob_ator1);
        double ator1 = st.prob_ator1 / (st.prob_ator0 + st.prob_ator1);

        for (unsigned iAcc=0; iAcc<accent_num-1; iAcc++)
        {
            vector<pair> toAccent;
            for (unsigned i=0; i<accentBranchNum; i++)
            {
                toAccent.push_back(pair(hmm.getStateNum() + 1 + i, 1.0 / (double)accentBranchNum));
            }

            hmm.addState(bs_r1, toAccent);

            unsigned r0Id = hmm.getStateNum() + accentBranchNum;
            unsigned r1Id = r0Id + 1 + phraseBranchNum;

            for (unsigned i=0; i<accentBranchNum; i++)
            {
                hmm.addState(bs_ac, vector<pair>{pair(r0Id, ator0), pair(r1Id, ator1)});
            }

            vector<pair> toPhrase;
            for (unsigned i=0; i<phraseBranchNum; i++)
            {
                toPhrase.push_back(pair(r0Id + 1 + i, 1.0/* /(double)phraseBranchNum*/));
            }
            hmm.addState(bs_r0, toPhrase);
            for (unsigned i=0; i<phraseBranchNum; i++)
            {
                hmm.addState(bs_ph, vector<pair>{pair(r1Id, 1.0)});
            }
        }

        unsigned r1Id = hmm.getStateNum();
        unsigned rinfId = r1Id + 1 + accentBranchNum;
        vector<pair> toAccent;
        for (unsigned i=0; i<accentBranchNum; i++)
        {
            toAccent.push_back(pair(r1Id+1+i, 1.0/(double)accentBranchNum));
        }
        hmm.addState(bs_r1, toAccent);
        for (unsigned i=0; i<accentBranchNum; i++)
        {
            hmm.addState(bs_ac, vector<pair>{pair(rinfId, 1.0)});
        }
        hmm.addState(bs_rinf, vector<pair>{pair(hmm.getStateNum() + 1, 1.0)});
        hmm.addState(bs_end, vector<pair>{});

        hmm.setInitialState(0);
        hmm.setFinalState(hmm.getStateNum() - 1);
        return hmm;
    }


    Hmm makeLoopFujisakiHmm(unsigned phraseBranchNum, unsigned accentBranchNum, const TransParams &st, unsigned margin_length)
    {
        Hmm hmm;

        if (phraseBranchNum < 1)
        {
            std::cerr << "No. of phrase states must be positive." << std::endl;
            exit(1);
        }
        if (accentBranchNum < 1)
        {
            std::cerr << "No. of accent states must be positive." << std::endl;
            exit(1);
        }
        vector<double> prob_rinf(margin_length, 1.0);

        BigState bs_rinf(prob_rinf, static_cast<int>(STATE_RESET_OTHER));
        BigState bs_r0(st.r0duration, static_cast<int>(STATE_RESET0));
        BigState bs_r1(st.r1duration, static_cast<int>(STATE_RESET1));

        BigState bs_ac(st.acduration, static_cast<int>(STATE_ACCENT));
        BigState bs_ph(st.phduration, static_cast<int>(STATE_PHRASE));
        BigState bs_begin(vector<double>(1, 1.0), static_cast<int>(STATE_BEGIN));
        BigState bs_end(vector<double>(1, 1.0), static_cast<int>(STATE_END));

        vector<pair> to_phrase;
        for (unsigned i_ph=0; i_ph<phraseBranchNum; i_ph++)
        {
            to_phrase.push_back(pair(6 + i_ph, 1.0 / (double)phraseBranchNum));
        }
        vector<pair> to_accent;
        for (unsigned i_ac=0; i_ac<accentBranchNum; i_ac++)
        {
            to_accent.push_back(pair(6 + phraseBranchNum + i_ac, 1.0 / (double)accentBranchNum));
        }

        hmm.addState(bs_begin, vector<pair>{pair(1, 1.0)});
        hmm.addState(bs_rinf, to_phrase);
        hmm.addState(bs_r0, to_phrase);
        hmm.addState(bs_r1, to_accent);
        hmm.addState(bs_rinf, vector<pair>{pair(5, 1.0)});
        hmm.addState(bs_end, vector<pair>{});
        for (unsigned i_ph=0; i_ph<phraseBranchNum; i_ph++)
        {
            hmm.addState(bs_ph, vector<pair>{pair(3, 1.0)});
        }

        double ator0 = st.prob_ator0 / (st.prob_ator0 + st.prob_ator1);
        double ator1 = st.prob_ator1 / (st.prob_ator0 + st.prob_ator1);
        for (unsigned i_ac=0; i_ac<accentBranchNum; i_ac++)
        {
            hmm.addState(bs_ac, vector<pair>{pair(2, ator0), pair(3, ator1), pair(4, 1.0)});
        }

        hmm.setInitialState(0);
        hmm.setFinalState(5);
        return hmm;
    }
}
