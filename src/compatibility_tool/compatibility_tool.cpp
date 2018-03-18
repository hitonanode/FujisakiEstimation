#include "compatibility_tool.hpp"
#include <algorithm>
#include <numeric>
#include "json.hpp"

#include "iofile.hpp"
#include "error_codes.hpp"

#include <iostream>

int main(int argc, char *argv[])
{
    cmdline::parser args;

    args.add<int>("mode", 'm', "action mode", true);
    args.add<std::string>("fs", 'f', "fs file name", false, "fs.txt");
    args.add<std::string>("lf0", 'l', "lf0 file name", false, "lf0.txt");
    args.add<std::string>("vuv", 'v', "vuv file name", false, "vuv.txt");
    args.add<std::string>("mup", 'p', "mup file name", false, "mup.txt");
    args.add<std::string>("mua", 'a', "mua file name", false, "mua.txt");
    args.add<std::string>("mub", 'b', "mub file name", false, "mub.txt");
    args.add<std::string>("out", 'o', "output file name", false, "out.json");
    
    args.parse_check(argc, argv);

    int mode = args.get<int>("mode");
    
    switch (mode)
    {
        case 0:
            {
                std::cout << "Raw data to InputData JSON file." << std::endl;
                stfmest::InputData inputdata = stfmest::rawDataFilesToInputData(args);
                jsonwrite(args.get<std::string>("out"), (nlohmann::json)inputdata);
            }
            break;
        case 1:
            {
                std::cout << "Raw data to std::vector<Commands> JSON file." << std::endl;
                std::vector<double> mup, mua;
                double fs;
                dlmread(args.get<std::string>("mup"), mup);
                dlmread(args.get<std::string>("mua"), mua);
                dlmread(args.get<std::string>("fs"), fs);
                std::vector<stfmest::FujisakiCommand> commands = stfmest::rawVectorDoubleToFujisakiCommands (
                    mup, mua, fs );
                jsonwrite(args.get<std::string>("out"), commands);
            }
            break;
        default:
            break;
    }
    std::cout << "Finished." << std::endl;
    return 0;
}

namespace stfmest
{
    InputData rawDataFilesToInputData (cmdline::parser &args)
    {
        std::vector<double> lf0, vuv, mup, mua;
        double fs, mub;
        if (!dlmread(args.get<std::string>("fs"), fs)
            || !dlmread(args.get<std::string>("lf0"), lf0)
            || !dlmread(args.get<std::string>("vuv"), vuv)
            || !dlmread(args.get<std::string>("mup"), mup)
            || !dlmread(args.get<std::string>("mua"), mua)
            || !dlmread(args.get<std::string>("mub"), mub)
            )
            {
                throw NOFILE_ERROR;
            }
        
        unsigned sigLen = lf0.size();
        if (vuv.size() != sigLen || mup.size() != sigLen || mua.size() != sigLen
            || sigLen < 1)
            {
                throw INPUT_VECTOR_SIZE_MISMATCH;
            }

        InputData id_;
        id_.fs = fs;
        id_.logf0 = lf0;
        id_.vuv = vuv;
        id_.initial_up = mup;
        id_.initial_ua = mua;
        id_.initial_mub = mub;
        return id_;
    }


    std::vector<FujisakiCommand> rawVectorDoubleToFujisakiCommands (
        const std::vector<double> &mup, 
        const std::vector<double> &mua,
        double fs
        )
    {
        unsigned frameLen = mup.size();
        std::cout << frameLen << std::endl;
        if (mua.size() != frameLen || frameLen < 1)
        {
            throw INPUT_VECTOR_SIZE_MISMATCH;
        }
        std::vector<FujisakiCommand> phraseCommands = _mux_to_fujisakicommands (
            mup, CMD_PHRASE, 3.0, fs );
        std::vector<FujisakiCommand> accentCommands = _mux_to_fujisakicommands (
            mua, CMD_ACCENT, 20.0, fs );
        
        std::vector<FujisakiCommand> commands = phraseCommands;
        commands.insert(commands.end(), accentCommands.begin(), accentCommands.end());

        std::sort(commands.begin(), commands.end());
        return commands;
    }


    std::vector<FujisakiCommand> _mux_to_fujisakicommands(
        const std::vector<double> &mux_,
        FilterType filtertype,
        double omega,
        double fs)
    {
        unsigned frameLen = mux_.size();

        std::vector<double> mux = mux_;
        mux.push_back(0.0);

        std::vector<FujisakiCommand> commands;
        unsigned commandOnsetFrame = 0;

        for (unsigned iFrame=0; iFrame<frameLen+1; iFrame++)
        {
            if (iFrame == 0)
            {
                if (mux[0] != 0.0)
                {
                    commandOnsetFrame = 0;
                }
                continue;
            }
            if (mux[iFrame] != mux[iFrame-1])
            {
                if (mux[iFrame-1] != 0.0)
                {
                    FujisakiCommand comm;
                    comm.filtertype = filtertype;
                    comm.onset = (double)commandOnsetFrame / fs;
                    comm.offset = (double)iFrame / fs;
                    comm.integratedAmplitude = std::accumulate(mux.begin()+commandOnsetFrame,
                                                            mux.begin()+iFrame, 0.0);
                    comm.omega = omega;
                    commands.push_back(comm);
                }
                if (mux[iFrame] != 0.0)
                {
                    commandOnsetFrame = iFrame;
                }
            }
        }

        return commands;
    }
}
