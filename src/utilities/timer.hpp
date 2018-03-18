// Measure running time of algorithms.
//
// 2018.03 Ryotaro Sato

#pragma once
#include <chrono>


namespace stfmest
{
    class Timer
    {
    public:
        Timer()
        {
            start();
        }

        inline void start()
        {
            starttime = std::chrono::system_clock::now();
        }

        inline double stop()
        {
            auto endtime = std::chrono::system_clock::now();
            duration_sec = endtime - starttime;
            return duration_sec.count();
        }

        inline double get() { return duration_sec.count(); }

        inline double lap()
        {
            auto endtime = std::chrono::system_clock::now();
            duration_sec = endtime - starttime;
            starttime = endtime;
            return duration_sec.count();
        }

    private:
        std::chrono::system_clock::time_point starttime;
        std::chrono::duration<double> duration_sec;
    };
}
