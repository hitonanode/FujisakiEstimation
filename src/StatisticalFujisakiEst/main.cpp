#include <iostream>
#include <string>
#include "cmdline.h"

#include "iofile.hpp"
#include "timer.hpp"
#include "em_estimation.hpp"
#include "evaluation.hpp"
#include "external_constraint.hpp"


stfmest::EstimationConfig config;
stfmest::TransParams hmmprob;
std::vector<stfmest::InputData> inputdata;
std::vector<stfmest::EstimationResult> results;


int main(int argc, char *argv[])
{
    cmdline::parser args;

    args.add<std::string>("config", 'c', "config file name", false, "config.json");
    args.add<std::string>("prob", 'p', "HMM probability file name", false, "hmmprob.json");
    args.add<std::string>("in", 'i', "input data file name", false, "input.json");
    args.add<std::string>("out", 'o', "output file name(optional)", false, "output.json");
    args.add<std::string>("truth", 't', "truth command data file name(optional)", false, "");
    args.add<std::string>("eval", 'e', "evaluation result file name", false, "evaluation.json");
    args.add<std::string>("const", 'x', "external constraint file name(optional)", false, "");
    args.parse_check(argc, argv);


    // Load config file
    config = jsonread(args.get<std::string>("config"));
    std::cout << "Loaded config data." << std::endl;

    // Load HMM probability file
    hmmprob = jsonread(args.get<std::string>("prob"));
    std::cout << "Loaded HMM probability data." << std::endl;

    // Load input data.
    nlohmann::json inputdatajson = jsonread(args.get<std::string>("in"));
    if (inputdatajson.is_array())
    {
        for (auto id_ : inputdatajson) inputdata.push_back(id_);
        std::cout << "Loaded " << inputdatajson.size() << " multiple input signals." << std::endl;
    }
    else
    {
        inputdata.push_back(inputdatajson);
        std::cout << "Loaded single input signal." << std::endl;
    }

    // Load ground truth command file if specified
    std::vector<std::vector<stfmest::FujisakiCommand> > groundtruth;
    if (args.get<std::string>("truth") != "")
    {
        nlohmann::json tmpj = jsonread(args.get<std::string>("truth"));
        for (auto i : tmpj) groundtruth.push_back(i);
        std::cout << "Loaded " << groundtruth.size() << " truth command patterns." << std::endl;
        if (groundtruth.size() < inputdata.size())
        {
            std::cerr << "Less than No. of input signal. Abort." << std::endl;
            exit(1);
        }
    }

    // Load external constraint file if specified
    nlohmann::json constraintjson;

    if (args.get<std::string>("const") != "")
    {
        nlohmann::json constraintJson_tmp = jsonread(args.get<std::string>("const"));
        constraintjson = constraintJson_tmp.at("constraintData");
        std::cout << "Loaded " << constraintjson.size() << " constraint data" << std::endl;
        if (constraintjson.size() < inputdata.size())
        {
			std::cerr << "Less than No. of input signal. Abort." << std::endl;
			exit(1);
        }
    }

    nlohmann::json resultarray;

    for (unsigned i=0; i<inputdata.size(); i++)
    {
        stfmest::InputData id_ = inputdata[i];
        if (args.get<std::string>("const") != "")
        {
            config.isHmmSerialized = true;
            nlohmann::json acc_const = constraintjson[i];
            int accentnum = acc_const.size();
            config.accentBigStateNum = accentnum;
        }
        stfmest::EmEstimationConstrained em(config);
        em.loadInputData(id_);

        std::vector<stfmest::StochasticCommandConstraint> constraintInfo;
        if (args.get<std::string>("const") != "")
            for (auto singleC : constraintjson[i])
            {
                constraintInfo.push_back(singleC);
                // std::cout << singleC << std::endl;
            }
        int status;
        if (config.enableLimitedDurationExtension)
        {
            em.loadTransparams(hmmprob, false);
            // std::cout << "Transparams loaded" << std::endl;
            em.emPreparation();
            // std::cout << "Preparation finished." << std::endl;
            if (args.get<std::string>("const") != "") em.imposeStochasticConst(constraintInfo);
            // std::cout << "Constraint introduced" << std::endl;
            status = em.validate();
            if (status)
            {
                std::cout << "[HMMprob modified]";
                em.loadTransparams(hmmprob, true);
                em.emPreparation();
                if (args.get<std::string>("const") != "") em.imposeStochasticConst(constraintInfo);
                status = em.validate();
            }
        }
        else
        {
            em.loadTransparams(hmmprob, true);
            em.emPreparation();
            if (args.get<std::string>("const") != "") em.imposeStochasticConst(constraintInfo);
        status = em.validate();
        }

        if (status)
        {
            std::cout << "EM prepared: " << status << std::endl;
            exit(1);
        }
        std::cout << "EM starts." << std::endl;
        stfmest::Timer t;
        t.start();
        em.launch();
        t.stop();
        // std::cout << "EM finished." << std::endl;
        results.push_back(em.getResult());
        std::cout << "[Input " << i << "] RMSE: " << results.back().rmse << " in " << t.get() << " sec. [End]" << std::endl;
        resultarray.push_back(results.back());
    }

    jsonwrite(args.get<std::string>("out"), resultarray);


    // evaluation
    if (args.get<std::string>("truth") != "")
    {
        std::vector<std::vector<std::pair<stfmest::FilterType, stfmest::CommandsCoincidenceResult> > > evalresults;
        for (unsigned i=0; i<inputdata.size(); i++)
        {
            evalresults.push_back(evaluateByDP(groundtruth[i], results[i].commands, 0.1, config.zeroThreshold));
        }

        std::vector<std::pair<stfmest::FilterType, stfmest::CommandsCoincidenceResult> > evalTotal{{stfmest::CMD_PHRASE, stfmest::CommandsCoincidenceResult()}, {stfmest::CMD_ACCENT, stfmest::CommandsCoincidenceResult()}};
        for (auto sentence : evalresults)
        {
            for (auto component : sentence)
            {
                unsigned i = 2;
                switch (component.first)
                {
                    case stfmest::CMD_PHRASE:
                        i = 0;
                        break;
                    case stfmest::CMD_ACCENT:
                        i = 1;
                        break;
                    default:
                        std::cerr << "Not found key." << std::endl;
                }
                evalTotal[i].second.allowedTimeLag = component.second.allowedTimeLag;
                evalTotal[i].second.matchedNum += component.second.matchedNum;
                evalTotal[i].second.totalNumInEstimated += component.second.totalNumInEstimated;
                evalTotal[i].second.totalNumInReference += component.second.totalNumInReference;
            }
        }
        evalresults.push_back(evalTotal);
        jsonwrite(args.get<std::string>("eval"), evalresults);
    }

    return 0;
}
