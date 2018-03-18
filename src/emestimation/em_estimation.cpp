#include "em_estimation.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include "fujisaki.hpp"
#include "error_codes.hpp"


template<class Type_>
using vector = std::vector<Type_>;


namespace stfmest
{
    void EmEstimation::loadInputData(InputData id_)
    {
        input = id_;
        frameNum = input.logf0.size();
    }


    void EmEstimation::loadTransparams(TransParams tp, bool regularize)
    {
        transparam = tp;
        if (regularize && config.durationExtensionFactor > 0.0)
        {
            transparam.regularize(config.durationExtensionFactor);
        }
    }


    void EmEstimation::_initHmm()
    {
        if (config.isHmmSerialized)
        {
            config.phraseBranchNum = 20;
            config.accentBranchNum = 20;
            hmm = makeSerializedFujisakiHmm(config.accentBigStateNum, transparam, 150, 160, config.phraseBranchNum, config.accentBranchNum);
        }
        else
        {
            // hmm = makeLoopFujisakiHmm(config.phraseBigStateNum, config.accentBigStateNum, transparam, frameNum/2);
            hmm = makeLoopFujisakiHmm(config.phraseBigStateNum, config.accentBigStateNum, transparam, transparam.r0duration.size());
        }

        phraseBigStateNum = hmm.countByStateType(STATE_PHRASE);
        accentBigStateNum = hmm.countByStateType(STATE_ACCENT);
        phraseNumToBigState.clear();
        accentNumToBigState.clear();

        for (unsigned i_big=0, bigStNum=hmm.getStateNum(); i_big<bigStNum; i_big++)
        {
            int attribute = hmm.getBigState(i_big).getAttribute();
            if (attribute == STATE_PHRASE)
            {
                phraseNumToBigState.push_back(i_big);
            }
            else if (attribute == STATE_ACCENT)
            {
                accentNumToBigState.push_back(i_big);
            }
        }
    }


    void EmEstimation::_initSmallStates()
    {
        smallStates.clear();
        transProbLog.clear();
        stateNum = 0u;

        vector<BigState> bigstates = hmm.getBigStates();

        // Calculate stateNum
        vector<unsigned> bigstatehead; // bigstatehead[i] = first small state corresponding to (i+1)th big state
        vector<unsigned> bigstatelen; // bigstatelen[i] = No. of small states corresponding to (i+1)th big state
        for (auto bs : bigstates)
        {
            bigstatehead.push_back(stateNum);
            unsigned len_tmp = bs.getSmallStateNum();
            stateNum += len_tmp;
            bigstatelen.push_back(len_tmp);
            // std::cout << bigstatehead.back() << "=>" << bigstatelen.back() << std::endl;
        }
        smallStates.resize(stateNum);
        // std::cout << "Small state num fixed: " << stateNum << std::endl;

        transProbLog = vector<std::map<int, double> >(stateNum, std::map<int, double>());


        for (unsigned i_bs=0; i_bs<hmm.getStateNum(); i_bs++){

            SmallState ss(bigstates[i_bs].getAttribute(), static_cast<int>(i_bs));

            for (unsigned i_ss=0; i_ss<bigstatelen[i_bs]; i_ss++){
                
                unsigned iSmallNow = bigstatehead[i_bs] + i_ss;

                if (i_ss < bigstatelen[i_bs] - 1)
                {
                    transProbLog[iSmallNow][iSmallNow+1] = 0.0;
                    ss.forwardConnects = {iSmallNow+1};
                }
                else
                {
                    // The last small state in each big state
                    ss.forwardConnects.clear();

                    for (auto nextBig : hmm.getTransition(i_bs))
                    {
                        if (nextBig.second <= 0.0) continue;
                        unsigned nextBigSt = nextBig.first;
                        double transProbTmp = nextBig.second;
                        vector<double> durationDist = bigstates[nextBigSt].getDurationDist();

                        for (unsigned itrNext=0; itrNext<bigstatelen[nextBigSt]; itrNext++)
                        {
                            unsigned iSmallNext = bigstatehead[nextBigSt] + itrNext;
                            double durationProbTmp = durationDist[bigstatelen[nextBigSt] - 1 - itrNext];
                            if (durationProbTmp <= 0.0) continue;

                            ss.forwardConnects.push_back(iSmallNext);
                            transProbLog[iSmallNow][iSmallNext] = log(transProbTmp * durationProbTmp);
                        }
                    }
                }
                smallStates[iSmallNow] = ss;
            }
        }

        // isStarting
        smallStates[bigstatehead[hmm.getInitialState()]].isStarting = true;

        // isEnding
        unsigned finalss = hmm.getFinalState();
        smallStates[bigstatehead[finalss] + bigstatelen[finalss] - 1].isEnding = true;

        // backwardConnects
        for (unsigned i_from=0; i_from<stateNum; i_from++)
        {
            for (auto i_to : smallStates[i_from].forwardConnects)
            {
                smallStates[i_to].backwardConnects.push_back(i_from);
            }
        }
    }


    void EmEstimation::_initReachableStateInfo()
    {
        
        isReachable = vector<vector<bool> >(frameNum, vector<bool>(stateNum, true));
        startingpoints.clear();
        endpoints.clear();

        // Initial frame setting
        for (unsigned i_st=0; i_st<stateNum; i_st++)
        {
            if (!smallStates[i_st].isStarting) isReachable[0][i_st] = false;
        }

        for (unsigned i_st=0; i_st<stateNum; i_st++)
        {
            if (smallStates[i_st].isStarting)
            {
                startingpoints.push_back(i_st);
            }
            else
            {
                isReachable[0][i_st] = false;
            }

            if (smallStates[i_st].isEnding)
            {
                if (isReachable[frameNum-1][i_st]) endpoints.push_back(i_st);
            }
            else
            {
                isReachable[frameNum-1][i_st] = false;
            }
        }
    }


    void EmEstimation::_updateReachableStateInfo()
    {
        startingpoints.clear();
        endpoints.clear();

        // Start forward search
        for (unsigned i_fr=1; i_fr<frameNum; i_fr++)
        {
            for (unsigned i_st = 0; i_st<stateNum; i_st++)
            {
                if (isReachable[i_fr][i_st])
                {
                    isReachable[i_fr][i_st] = false;
                    for (auto st_prev : smallStates[i_st].backwardConnects)
                    {
                        if (isReachable[i_fr-1][st_prev])
                        {
                            isReachable[i_fr][i_st] = true;
                            break;
                        }
                    }
                }
            }
        }

        // Next, backward search
        for (int i_fr=frameNum-2; i_fr>=0; i_fr--)
        {
            for (unsigned i_st=0; i_st<stateNum; i_st++)
            {
                if (isReachable[i_fr][i_st])
                {
                    isReachable[i_fr][i_st] = false;
                    for (auto st_next : smallStates[i_st].forwardConnects)
                    {
                        if (isReachable[i_fr+1][st_next])
                        {
                            isReachable[i_fr][i_st] = true;
                            break;
                        }
                    }
                }
            }
        }

        for (unsigned i_st=0; i_st<stateNum; i_st++)
        {
            if (isReachable[0][i_st]) startingpoints.push_back(i_st);
            if (isReachable[frameNum-1][i_st]) endpoints.push_back(i_st);
        }

        return;    
    }


    void EmEstimation::_initEmParameters()
    {
        alpha = config.defaultAlpha;
        beta = config.defaultBeta;
        Gp = vector<double>(frameNum, 0.0);
        Ga = vector<double>(frameNum, 0.0);
        invsigma2_p = 1.0 / config.defaultSigmap2;
        invsigma2_a = 1.0 / config.defaultSigmaa2;
        invsigma2_n = vector<double>(frameNum, 0.0);

        for (unsigned i_fr=0; i_fr<frameNum; i_fr++)
        {
            double tsec = (double)i_fr / input.fs;
            Gp[i_fr] = impulse_response(alpha, tsec) / input.fs;
            Ga[i_fr] = impulse_response(beta, tsec) / input.fs;
            invsigma2_n[i_fr] = input.vuv[i_fr] > config.zeroThreshold
                                ? 1.0 / config.defaultSigman2_voiced
                                : 1.0 / config.defaultSigman2_unvoiced;
        }
    }

    void EmEstimation::_viterbiAlgorithm()
    {
        // Optimal probs.
        delta = vector<vector<double> >(frameNum, vector<double>(stateNum, -config.inf));
        // previous small state for each frame/state.
        s_before = vector<vector<unsigned> >(frameNum, vector<unsigned>(stateNum, stateNum));

        // Setting delta at initial frame
        for (unsigned i_st : startingpoints) delta[0][i_st] = _emissionProbLog(0, i_st);
        
        // The Viterbi Algorithm(main part)
        for (unsigned i_fr=1; i_fr<frameNum; i_fr++)
        {
            for (unsigned i_st = 0; i_st<stateNum; i_st++)
            {
                if (!isReachable[i_fr][i_st]) continue;

                double edgeMax = 0.0;
                unsigned tempPreviousState = stateNum;
                for (auto st_prev : smallStates[i_st].backwardConnects)
                {
                    if (!isReachable[i_fr-1][st_prev]) continue;
                    double edge_tmp = delta[i_fr-1][st_prev] + _transitionProbLog(st_prev, i_st);

                    if (tempPreviousState == stateNum || edge_tmp - edgeMax > std::abs(edgeMax) * config.zeroThreshold)
                    {
                        edgeMax = edge_tmp;
                        tempPreviousState = st_prev;
                    }
                }
                if (tempPreviousState != stateNum)
                {
                    s_before[i_fr][i_st] = tempPreviousState;
                    delta[i_fr][i_st] = edgeMax + _emissionProbLog(i_fr, i_st);
                }
            }
        }
        // Calculating  isReachable, s_before, delta  finished.
        unsigned optimalLastState = stateNum;
        double deltaMax = 0.0;
        for (unsigned i_st=0; i_st<stateNum; i_st++)
        {
            if (smallStates[i_st].isEnding && isReachable[frameNum-1][i_st])
            {
                if (optimalLastState == stateNum || delta[frameNum-1][i_st] > deltaMax)
                {
                    optimalLastState = i_st;
                    deltaMax = delta[frameNum-1][i_st];
                }
            }
        }
        
        if (optimalLastState == stateNum)
        {
            std::cerr << "Viterbi failed." << std::endl;
            exit(1);
        }
        s[frameNum-1] = optimalLastState;
        for (int i_fr=frameNum-2; i_fr>=0; i_fr--) s[i_fr] = s_before[i_fr+1][s[i_fr+1]];

        return;
    }


    void EmEstimation::_initEmVariables()
    {
        lambda_p = vector<vector<double> >(frameNum, vector<double>(frameNum, 0.0));
        lambda_a = vector<vector<double> >(frameNum, vector<double>(frameNum, 0.0));
        s = vector<unsigned>(frameNum, stateNum);
        if (config.isHardEmEnabled)
        {
            gamma.clear();
        }
        else
        {
            gamma = vector<vector<double> >(frameNum, vector<double>(frameNum, 0.0));
        }
        up.clear();
        // up = input.initial_up;
        // for (unsigned i=0; i<up.size(); i++) up[i] = std::max(up[i], config.regularizerOffset);
        ua = input.initial_ua;
        for (unsigned i=0; i<ua.size(); i++) ua[i] = std::max(ua[i], config.regularizerOffset);
        mub = input.initial_mub;
        Cp.clear();
        Ca.clear();
        
        // Initialize up & Cp
        up.resize(frameNum, config.regularizerOffset);
        Cp.resize(hmm.getStateNum(), 0.0);
        
        for (unsigned i=0; i<phraseNumToBigState.size(); i++)
        {
            if (config.isHmmSerialized)
            {
                Cp[phraseNumToBigState[i]] = (double)(i%config.phraseBranchNum + 1) * 1.25 * 20. / config.phraseBranchNum;
            }
            else
            {
                Cp[phraseNumToBigState[i]] = (double)(i+1) * 1.25;
            }
        }
        unsigned phrasecount = 0;
        for (unsigned i_fr=0; i_fr<frameNum; i_fr++)
        {
            if (input.initial_up[i_fr] > config.zeroThreshold)
            {
                double amplitude = input.initial_up[i_fr] / (double)(config.phraseOnDuration);
                for (int i=0; i<std::min((int)i_fr+1, config.phraseOnDuration); i++)
                {
                    up[i_fr-i] = amplitude;
                }
                if (phrasecount >= phraseBigStateNum)
                {
                    std::cerr << "Too many phrase commands in initial value." << std::endl;
                }
                else
                {
                    // Cp[phraseNumToBigState[phrasecount]] = amplitude;
                    ++phrasecount;
                }
            }
        }
        
        // Initialize Ca
        Ca.resize(hmm.getStateNum(), 0.0);
        for (unsigned i=0; i<accentNumToBigState.size(); i++)
        {
            if (config.isHmmSerialized)
            {
                Ca[accentNumToBigState[i]] = (double)(i%config.accentBranchNum + 1) / config.accentBranchNum;
            }
            else
            {
                Ca[accentNumToBigState[i]] = ((double)i+1) / accentNumToBigState.size();
            }
        }/*
        unsigned accentcount = 0;
        for (unsigned i_fr=1; i_fr<frameNum; i_fr++)
        {
            double amplitude = input.initial_ua[i_fr];
            if (amplitude > config.zeroThreshold && amplitude != input.initial_ua[i_fr-1])
            {
                if (accentcount >= accentBigStateNum)
                {
                    std::cerr << "Too many accent commands in initial value." << std::endl;
                    break;
                }
                else
                {
                    Ca[accentNumToBigState[accentcount]] = amplitude;
                    ++accentcount;
                }
            }
        }*/
        return;
    }


    void EmEstimation::emPreparation()
    {
        _initHmm();
        // std::cout << "HMM initialized." << std::endl;
        _initSmallStates();
        // std::cout << "ss initialized." << std::endl;
        _initReachableStateInfo();
        _updateReachableStateInfo();
        // std::cout << "rsi initialized." << std::endl;
        _initEmParameters();
        // std::cout << "EM parameters initialized." << std::endl;
        _initEmVariables();
        // std::cout << "EM variables initialized." << std::endl;
        if (constraintProbLog.size() != frameNum
            || constraintProbLog[0].size() != stateNum)
            constraintProbLog = vector<vector<double> >(frameNum, vector<double>(stateNum, 0.0));
        // std::cout << "constraintProbLog initialized." << std::endl;
    }


    bool EmEstimation::_updateLambda()
    {
        // Update rule of the lambda's
        //
        // As you can see from the update rule (83-84) in [Kameoka+, 2015],
        // any resultant lambda^p and lambda^a's must NOT be zero,
        // so we add the artificial regularization term with keeping the condition (78).

        for (unsigned k=0; k<frameNum; k++)
        {
            double denominator = 0.0;
            for (unsigned ll=0; ll<=k; ll++) // '<=', not '<'
            {
                denominator += Gp[k-ll] * up[ll] + Ga[k-ll] * ua[ll]; // + 2.0 * config.regularizerOffset;
            }
            denominator += mub;

            for (unsigned l=0; l<=k; l++)
            {
                lambda_p[k][l] = (Gp[k-l] * up[l] /*+ config.regularizerOffset*/) / denominator;
                lambda_a[k][l] = (Ga[k-l] * ua[l] /*+ config.regularizerOffset*/) / denominator;
            }
        }
        return true;
    }


    bool EmEstimation::_updateUpUaHard()
    {
        _u_update_function_hard(up, Gp, Cp, lambda_p, invsigma2_p);
        _u_update_function_hard(ua, Ga, Ca, lambda_a, invsigma2_a);
        return true;
    }

    inline bool EmEstimation::_u_update_function_hard(vector<double> &ux, const vector<double> &Gx, const vector<double> &Cx, const vector<vector<double> > &lambda_x, double invsigma2_x)
    {
        for (unsigned l=0; l<frameNum; l++) {

            double denominator = invsigma2_x;
            double numerator = Cx[smallStates[s[l]].bigstateId] * invsigma2_x;

            for (unsigned k=l; k<frameNum; k++)
            {
                if (lambda_x[k][l] >= config.zeroThreshold){
                    denominator += Gx[k-l] * Gx[k-l] * invsigma2_n[k] / lambda_x[k][l];
                }
                numerator += (input.logf0[k]/* - mub*/) * Gx[k-l] * invsigma2_n[k];
            }
            // if (denominator > config.zeroThreshold)
            // {
                // ux[l] = std::max(numerator / denominator, 0.0);
            // }
            ux[l] = numerator / denominator;
        }
        return true;
    }


    bool EmEstimation::_updateCpCaHard()
    {
        _c_update_function_hard(Cp, up, invsigma2_p, STATE_PHRASE);
        _c_update_function_hard(Ca, ua, invsigma2_a, STATE_ACCENT);
        return true;
    }

    inline bool EmEstimation::_c_update_function_hard(vector<double> &Cx, const vector<double> &ux, double invsigma2_x, int attribute)
    {
        unsigned bigStateNum = hmm.getStateNum();

        vector<double> numerator(bigStateNum, 0.0);
        vector<double> denominator(bigStateNum, 0.0);

        for (unsigned i_fr=0; i_fr<frameNum; ++i_fr)
        {
            if (smallStates[s[i_fr]].attribute == attribute)
            {
                numerator[smallStates[s[i_fr]].bigstateId] += ux[i_fr] * invsigma2_x;
                denominator[smallStates[s[i_fr]].bigstateId] += invsigma2_x;
            }
        }

        for (unsigned i_x=0; i_x<bigStateNum; ++i_x)
        {
            if (denominator[i_x] >= config.zeroThreshold && numerator[i_x] >= config.zeroThreshold)
            {
                Cx[i_x] = numerator[i_x] / denominator[i_x];
            }
        }
        return true;
    }


    void EmEstimation::_hardMstep()
    {
        for (int iter=0; iter<config.mstepUpdateNumPerIteration; iter++)
        {
            _updateLambda();
            _updateUpUaHard();
            _updateCpCaHard();
        }
    }

    void EmEstimation::_iterateHardEm()
    {
        for (int iter=0; iter<config.iterationNum; iter++)
        {
            // std::cout << "Viterbi " << iter << std::endl;
            _viterbiAlgorithm();
            // std::cout << "hard M " << iter << std::endl;
            _hardMstep();
            // std::cout << "Perturb " << iter << std::endl;
            _perturbCommands();
        }
    }


    bool EmEstimation::launch()
    {
        _iterateHardEm();
        return true;
    }


    int EmEstimation::_validateBeforeEm()
    {
        // Check vectors' size
        if (frameNum < 1
            || input.logf0.size() != frameNum
            || input.vuv.size() != frameNum
            || input.initial_up.size() != frameNum
            || input.initial_ua.size() != frameNum)
        {
            return INPUT_VECTOR_SIZE_MISMATCH;
        }

        // Check HMM
        if (phraseBigStateNum < 1
            || accentBigStateNum < 1
            || hmm.getBigStates().size() != hmm.getStateNum())
        {
            return HMM_SETUP_ERROR;
        }

        // Check config
        if ((!config.isHmmSerialized && config.phraseBigStateNum<1)
            || config.accentBigStateNum < 1
            || config.iterationNum < 0
            || config.mstepUpdateNumPerIteration < 0
            || config.perturbSearchWidth < 0
            || config.defaultAlpha < 0
            || config.defaultBeta < 0
            || config.defaultSigmap2 <= 0
            || config.defaultSigmaa2 <= 0
            || config.defaultSigman2_voiced <= 0
            || config.defaultSigman2_unvoiced <= 0
            || config.defaultSigman2_unvoiced <= 0
            || config.regularizerOffset < 0
            || config.zeroThreshold < 0
            || config.inf <= 0
            )
        {
            return CONFIG_VALUE_INVALID;
        }

        // Check transparam
        if (transparam.phduration.size() < 1
            || transparam.acduration.size() < 1
            || transparam.r0duration.size() < 1
            || transparam.r1duration.size() < 1)
        {
            return TRANSPARAM_VECTOR_SIZE_INVALID;
        }
        if (transparam.prob_ator0 < 0 || transparam.prob_ator1<0)
        {
            return TRANSPARAM_PROB_INVALID;
        }
        for (auto i:transparam.phduration) if (i<0.0) return TRANSPARAM_PROB_INVALID;
        for (auto i:transparam.acduration) if (i<0.0) return TRANSPARAM_PROB_INVALID;
        for (auto i:transparam.r0duration) if (i<0.0) return TRANSPARAM_PROB_INVALID;
        for (auto i:transparam.r1duration) if (i<0.0) return TRANSPARAM_PROB_INVALID;


        // Check Consistency
        if ((int)transparam.phduration.size() != config.phraseOnDuration)
        {
            return PHASE_DURATION_MISMATCH;
        }
        if (smallStates.size() != stateNum) return CONSISTENCY_ERROR;


        // Check preparation
        if (Gp.size() != frameNum || Ga.size() != frameNum || invsigma2_n.size() != frameNum) return CONSISTENCY_ERROR;

        // No solution
        if (startingpoints.size() < 1 || endpoints.size() < 1) return NOSOLUTION_ERROR;

        // All check passed:
        return NO_ERROR;
    }

    std::vector<FujisakiCommand> EmEstimation::_getCommands()
    {
        std::vector<FujisakiCommand> cmds;
        int bigstatenum_before = hmm.getInitialState();
        int bigstatenum = bigstatenum_before;
        bool isCommandOnNow = false;
        for (unsigned i_fr=0; i_fr<frameNum+1; i_fr++)
        {
            if (i_fr == frameNum) {
                bigstatenum = hmm.getFinalState();
            }
            else {
                bigstatenum = smallStates[s[i_fr]].bigstateId;
            }
            if (bigstatenum != bigstatenum_before)
            {
                if (isCommandOnNow)
                {
                    int oldattr = hmm.getBigState(bigstatenum_before).getAttribute();
                    switch (oldattr)
                    {
                        case STATE_PHRASE:
                            cmds.back().offset = (double)i_fr / input.fs;
                            cmds.back().integratedAmplitude = (cmds.back().offset - cmds.back().onset ) * Cp[bigstatenum_before];
                            if (std::abs(Cp[bigstatenum_before]) < config.zeroThreshold) cmds.pop_back();
                            break;
                        case STATE_ACCENT:
                            cmds.back().offset = (double)i_fr / input.fs;
                            cmds.back().integratedAmplitude = (cmds.back().offset - cmds.back().onset ) * Ca[bigstatenum_before];
                            if (std::abs(Ca[bigstatenum_before]) < config.zeroThreshold) cmds.pop_back();
                            break;
                        case STATE_BEGIN:
                        case STATE_END:
                        case STATE_RESET0:
                        case STATE_RESET1:
                        case STATE_RESET_OTHER:
                            break;
                        default:
                            std::cerr << "Invalid attribute " << oldattr << " is set for the state." << std::endl;
                            exit(1);
                    }
                    isCommandOnNow = false;
                }

                int newattr = hmm.getBigState(bigstatenum).getAttribute();
                switch (newattr)
                {
                    case STATE_PHRASE:
                        isCommandOnNow = true;
                        cmds.push_back(FujisakiCommand(CMD_PHRASE, (double)i_fr/input.fs,
                                            0.0, 0.0, config.defaultAlpha));
                        break;
                    case STATE_ACCENT:
                        isCommandOnNow = true;
                        cmds.push_back(FujisakiCommand(CMD_ACCENT, (double)i_fr/input.fs,
                                            0.0, 0.0, config.defaultBeta));
                        break;
                    case STATE_BEGIN:
                    case STATE_END:
                    case STATE_RESET0:
                    case STATE_RESET1:
                    case STATE_RESET_OTHER:
                        break;
                    default:
                        std::cerr << "Invalid attribute " << newattr << " is set for the state." << std::endl;
                        exit(1);
                }
            }
            bigstatenum_before = bigstatenum;        
        }
        return cmds;
    }


    inline double EmEstimation::_ux_observedlf0_distance(std::vector<double> &up_, std::vector<double> &ua_)
    {
        std::vector<double> lf0regen(frameNum, mub);
        for (unsigned k=0; k<frameNum; k++) {
            for (unsigned l=0; l<=k; l++) {
                lf0regen[k] += up_[l] * Gp[k-l] + ua_[l] * Ga[k-l];
            }
        }

        double logdist2 = 0.0;
        for (unsigned k=0; k<frameNum; k++)
        {
            logdist2 += (input.logf0[k] - lf0regen[k]) * (input.logf0[k] - lf0regen[k]) * invsigma2_n[k];
        }
        return logdist2;
    }


    EstimationResult EmEstimation::getResult()
    {

        _viterbiAlgorithm();

        EstimationResult er;
        // Derive mup&mua
        er.mup = std::vector<double>(frameNum, 0.0);
        er.mua = std::vector<double>(frameNum, 0.0);
        er.mub = mub;
        for (unsigned i_fr=0; i_fr<frameNum; i_fr++)
        {
            int bigstatenum = smallStates[s[i_fr]].bigstateId;
            er.mup[i_fr] = Cp[bigstatenum];
            er.mua[i_fr] = Ca[bigstatenum];
        }

        er.Cp = Cp;
        er.Ca = Ca;
        er.bigs = std::vector<int>(frameNum);
        for (unsigned i=0; i<er.bigs.size(); i++) er.bigs[i] = smallStates[s[i]].bigstateId;

        er.commands = _getCommands();
        er.regeneratedlf0 = criticalfilter(er.commands, mub, input.fs, frameNum);
        er.rmse = rmse(input.logf0, er.regeneratedlf0, input.vuv, config.zeroThreshold);
        er.voicedFrameNum = 0;
        for (auto vuv : input.vuv) if (vuv > config.zeroThreshold) ++(er.voicedFrameNum);
        return er;
    }

    inline double Ylikelihood (
        const std::vector<double> &y,
        const std::vector<double> &invsigma2_n,
        const std::vector<double> &up,
        const std::vector<double> &ua,
        const std::vector<double> &Gp,
        const std::vector<double> &Ga,
        double mub
    )
    {
        double ans = 0;
        unsigned nFrame = y.size();
        std::vector<double> yRegen(nFrame, 0.0);
        
        for (unsigned k=0; k<nFrame; k++)
        {
            for (unsigned l=0; l<=k; l++) yRegen[k] += up[l] * Gp[k-l] + ua[l] * Ga[k-l];
            yRegen[k] += mub;
            ans += -(y[k] - yRegen[k]) * (y[k] - yRegen[k]) * 0.5 * invsigma2_n[k];
        }
        return ans;
    }

    void EmEstimation::_perturbCommands()
    {
        std::vector<FujisakiCommand> commands = _getCommands();

        for (unsigned i=0, n=commands.size(); i<n; i++)
        {
            if (commands[i].filtertype == CMD_PHRASE) continue;
            int prevFrame = 0;
            int nextFrame = frameNum;
            if ( i>0 ) prevFrame = (int)std::round(commands[i-1].offset * input.fs) + 1;
            if ( i<n-1 ) nextFrame = (int)std::round(commands[i+1].onset * input.fs);

            double minDivergence = _ux_observedlf0_distance(up, ua);
            bool isUpdateNeeded = false;
            int frameDiffUpdate = 0;

            int frame_comm_start_ref = (int)std::round(commands[i].onset * input.fs);
            int frame_comm_end_ref = (int)std::round(commands[i].offset * input.fs);
            std::vector<double> up_tmp(frameNum);
            std::vector<double> ua_tmp(frameNum);

            for (int fr = - config.perturbSearchWidth; fr <= config.perturbSearchWidth; fr++)
            {
                if (fr == 0) continue;
                if ( prevFrame >= frame_comm_start_ref  + fr ||
                    nextFrame <= frame_comm_end_ref + fr)
                {
                    continue;
                }
                up_tmp = up;
                ua_tmp = ua;
                
                for (int j=frame_comm_start_ref; j<frame_comm_end_ref; j++)
                {
                    up_tmp[j] = config.regularizerOffset;
                    ua_tmp[j] = config.regularizerOffset;
                }
                for (int j=frame_comm_start_ref; j<frame_comm_end_ref; j++)
                {
                    up_tmp[j + fr] = up[j];
                    ua_tmp[j + fr] = ua[j];
                }
                double dist_tmp = _ux_observedlf0_distance(up_tmp, ua_tmp);
                if (dist_tmp < minDivergence)
                {
                    minDivergence = dist_tmp;
                    isUpdateNeeded = true;
                    frameDiffUpdate = fr;
                }
            }
            // update up&ua
            if (isUpdateNeeded)
            {
                up_tmp = up;
                ua_tmp = ua;
                for (int j=frame_comm_start_ref; j<frame_comm_end_ref; j++)
                {
                    up[j] = config.regularizerOffset;
                    ua[j] = config.regularizerOffset;
                }
                for (int j=frame_comm_start_ref; j<frame_comm_end_ref; j++)
                {
                    up[j + frameDiffUpdate] = up_tmp[j];
                    ua[j + frameDiffUpdate] = ua_tmp[j];
                }
            }
        }
    
    }
}