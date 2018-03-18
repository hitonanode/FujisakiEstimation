// Estimation algorithm
//
// 2018.03 Ryotaro Sato

#pragma once
#include <vector>
#include "hmm_fujisaki.hpp"
#include "input_data.hpp"
#include "estimation_config.hpp"
#include "fujisaki.hpp"
#include "small_state.hpp"
#include "estimation_result.hpp"


namespace stfmest
{
    class EmEstimation
    {
    protected:
        InputData input;
        unsigned frameNum; // frame No. of input signal
        EstimationConfig config;
        TransParams transparam;

        Hmm hmm;
        unsigned phraseBigStateNum;
        unsigned accentBigStateNum;
        std::vector<int> phraseNumToBigState; // ph...[i] == big state no. of (i+1)th phrase-on big state
        std::vector<int> accentNumToBigState; // ac...[i] == big state no. of (i+1)th accent-on big state


    protected:
        unsigned stateNum; // No. of small states generated in this->hmm
        std::vector<SmallState> smallStates;
        std::vector<std::map<int, double> > transProbLog; // log trans.prob. betw. small states


        // Preparation for executing the EM algorithm
    protected:
        std::vector<std::vector<bool> > isReachable; // Permit on each state at each frame, or not
    private:
        std::vector<unsigned> startingpoints; // The No's of small states candidate for the initial state.
        std::vector<unsigned> endpoints;
        
        double alpha;
        double beta;
        std::vector<double> Gp;
        std::vector<double> Ga;
        double invsigma2_p;
        double invsigma2_a;
        std::vector<double> invsigma2_n;


        // Parameters used in EM algorithm.
        std::vector<std::vector<double> > lambda_p;
        std::vector<std::vector<double> > lambda_a;
        std::vector<unsigned> s; // for Hard EM
        std::vector<std::vector<double> > gamma; // for Soft EM
        std::vector<double> up;
        std::vector<double> ua;
        double mub;
        std::vector<double> Cp;
        std::vector<double> Ca;
        std::vector<std::vector<double> > delta;
        std::vector<std::vector<unsigned> > s_before;

        // external constraint 
    protected:
        std::vector<std::vector<double> > constraintProbLog;


        // For Viterbi algorithm
        inline double _emissionProbLog(unsigned frame, unsigned smallstatenum)
        {
            return _emissionProbLogDefault(frame, smallstatenum) + constraintProbLog[frame][smallstatenum];
        }
        
        inline double _emissionProbLogDefault(unsigned frame, unsigned smallstatenum)
        {
            int bigstatenum = smallStates[smallstatenum].bigstateId;
            double mup = Cp[bigstatenum];
            double mua = Ca[bigstatenum];
            return -0.5 * (up[frame] - mup) * (up[frame] - mup) * invsigma2_p 
                -0.5 * (ua[frame] - mua) * (ua[frame] - mua) * invsigma2_a;
        }

        inline double _transitionProbLog(unsigned state_prev, unsigned state_now)
        {
            // if (transProbLog[state_prev].count(state_now))
                return transProbLog[state_prev][state_now];
        }

        void _initHmm();
        void _initSmallStates();
        void _initReachableStateInfo();
    protected:
        void _updateReachableStateInfo();
    private:
        void _initEmParameters();
        void _initEmVariables();
        int _validateBeforeEm();
        void _iterateHardEm();

        // E step
        void _viterbiAlgorithm(); // update s

        // M step
        void _hardMstep();

        bool _updateLambda();
        bool _updateUpUaHard();
        inline bool _u_update_function_hard(std::vector<double> &ux, const std::vector<double> &Gx, const std::vector<double> &Cx, const std::vector<std::vector<double> > &lambda_x, double invsigma2_x);
        bool _updateCpCaHard();
        inline bool _c_update_function_hard(std::vector<double> &Cx, const std::vector<double> &ux, double invsigma2_x, int attribute);

        std::vector<FujisakiCommand> _getCommands();
        void _perturbCommands();
        inline double _ux_observedlf0_distance(std::vector<double> &up_, std::vector<double> &ua_);

    public:
        EmEstimation(EstimationConfig ec): config(ec) {}
        void loadInputData(InputData id_); 
        void loadTransparams(TransParams tp, bool regularize);
        inline TransParams getTransParams() { return transparam; }
        void emPreparation(); // Preparation for EM algorithm
        inline int validate(){ return _validateBeforeEm(); }
        bool launch(); // Launch EM.
        EstimationResult getResult();

        std::vector<SmallState> getSmallStates() { return smallStates; }
    };
}