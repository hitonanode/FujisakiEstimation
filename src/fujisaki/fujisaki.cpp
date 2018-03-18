#include <algorithm>
#include <iostream>
#include <cmath>
#include "fujisaki.hpp"


namespace stfmest
{
    std::vector<double> criticalfilter(const FujisakiCommand &command, double fs, int frameNum)
    {
        std::vector<double> output(frameNum, 0.0);
        
        if (command.offset < command.onset)
        {
            std::cerr << "Command onset/offset time invalid: " << command.onset << " " << command.offset << std::endl;
            exit(1);
        }

        if (command.omega < 1e-9)
        {
            std::cerr << "Command time constant too small: " << command.omega << std::endl;
            exit(1);
        }

        if ((command.offset - command.onset) * command.omega < 0.01)
        {
            // For impluse-like command
            for (unsigned i=0; i<output.size(); i++)
            {
                double time_ = (double)i / fs;
                output[i] = impulse_response(command.omega, time_ - command.onset)
                            * command.integratedAmplitude;
            }
        }
        else
        {
            // For rectangular-like command
            double amplitude = command.integratedAmplitude / (command.offset - command.onset);

            for (unsigned i=0; i<output.size(); i++)
            {
                double time_ = (double)i / fs;
                output[i] = (step_response(command.omega, time_ - command.onset)
                        - step_response(command.omega, time_ - command.offset))
                        * amplitude;
            }
        }

        return output;
    }


    std::vector<double> criticalfilter(const std::vector<FujisakiCommand> &commands,
                                    double mub, double fs, int frameNum)
    {
        std::vector<double> sum(frameNum, mub);
        for (auto comm : commands)
        {
            std::vector<double> tmp = criticalfilter(comm, fs, frameNum);
            std::transform(sum.begin(), sum.end(), tmp.begin(), sum.begin(), std::plus<double>());
        }
        return sum;
    }

    double rmse(const std::vector<double> &lf0a, const std::vector<double> &lf0b,
                const std::vector<double> &vuv, double vuvThres)
    {
        if (lf0a.size() != lf0b.size() || lf0a.size() != vuv.size())
        {
            std::cerr << "Input length not matched: " << lf0a.size() << " " << lf0b.size() << " " << vuv.size() << std::endl;
            exit(1);
        }
        double squaredError = 0.0;
        int voicedNum = 0;  
        for (unsigned i=0; i<lf0a.size(); i++)
        {
            if (vuv[i] > vuvThres)
            {
                squaredError += (lf0a[i] - lf0b[i]) * (lf0a[i] - lf0b[i]);
                ++voicedNum;
            }
        }
        if (voicedNum == 0)
        {
            std::cerr << "No voiced segment in calculating RMSE." << std::endl;
            return 1E20;
        }
        return std::sqrt(squaredError / voicedNum);
    }
}
