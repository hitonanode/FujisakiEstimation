Statistical Fujisaki Model Command Estimation
====

This is the prosodic parameter estimation algorithm
implemented in C++, which is proposed at [Sato+, 2017].


## Build

This program uses CMake and the function is checked 
only in the environment: Visual Studio 2017 with Microsoft Windows.
In the case of other environments,
you should modify the 'CMakeLists.txt'.


## Run

An example for demonstration is prepared in the 
'demo' directory. Put the generated bin file
'StatisticalFujisakiEst.exe' into this directory and run it to see that the estimation result is
output to the file 'output.json'.

The command line options for specifying input/output
files are shown by the option '-h'.


## License
This repository can be used only for
academic research. For this purpose,
you can copy and modify this program freely.

The program partially uses [nlohmann/json](https://github.com/nlohmann/json)
and [tanakh/cmdline](https://github.com/tanakh/cmdline).
See the license of these repositories, too.


## References

[1] R. Sato, H. Kameoka, K. Kashino,
"Fast algorithm for statistical phrase/accent
command estimation based on generative model
incorporating spectral features,"
*ICASSP*, pp.5595-5599, 2017.