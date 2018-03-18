#include "iofile.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

template <typename T>
bool dlmread(const std::string &filename, std::vector<std::vector<T> > &Mat)
{

    Mat.resize(0);

    std::ifstream ifs(filename.c_str());
    if (!ifs.is_open())
        return false;
    
    std::string line;
    while (getline(ifs, line)){
        std::vector<T> latestraw(0);
        std::istringstream stream(line);
        std::string tmp;
        while (getline(stream, tmp, ' '))
            latestraw.push_back(stod(tmp));
        Mat.push_back(latestraw);
    }
    return true;
}
template bool dlmread(const std::string &filename, std::vector<std::vector<double> > &Mat);

bool dlmread(const std::string &filename, std::vector<double> &Vec)
{

    Vec.resize(0);

    std::ifstream ifs(filename.c_str());
    if (!ifs.is_open())
        return false;

    std::string line;
    while (getline(ifs, line)){
        if (line==(std::string)"")
            return false;
        Vec.push_back(stod(line));
    }

    return true;
}


bool dlmread(const std::string &filename, double &Val)
{

    Val = 0.0;

    std::ifstream ifs(filename.c_str());
    std::string line;
    if (!ifs.is_open() || !getline(ifs, line))
        return false;

    Val = stod(line);

    return true;
}

template <typename T>
bool dlmwrite(const std::string &filename, std::vector<std::vector<T> > &Mat)
{
    std::ofstream ofs(filename.c_str());
    if (!ofs.is_open())
        return false;

    for (auto& raw : Mat){
        T tmp = raw.back();
        raw.pop_back();
        for (auto& x : raw)
            ofs << x << " ";
        ofs << tmp << std::endl;
    }

    return true;
}
template bool dlmwrite(const std::string &filename, std::vector<std::vector<double> > &Mat);
template bool dlmwrite(const std::string &filename, std::vector<std::vector<unsigned> > &Mat);

template <typename T>
bool dlmwrite(const std::string &filename, std::vector<T> &Vec)
{
    std::ofstream ofs(filename.c_str());
    if (!ofs.is_open())
        return false;

    for (auto& x : Vec)
        ofs << x << std::endl;

    return true;
}
template bool dlmwrite(const std::string &filename, std::vector<double> &Vec);
template bool dlmwrite(const std::string &filename, std::vector<std::string> &Vec);

template <typename T>
bool dlmwrite(const std::string &filename, T &Val)
{
    std::ofstream ofs(filename.c_str());
    if (!ofs.is_open())
        return false;

    ofs << Val << std::endl;

    return true;
}
template bool dlmwrite(const std::string &filename, double &Val);
template bool dlmwrite(const std::string &filename, std::string &Val);


nlohmann::json jsonread(const std::string &filename)
{
    nlohmann::json j;
    std::ifstream ifs(filename);
    ifs >> j;
    return j;
}

bool jsonwrite(const std::string &filename, const nlohmann::json &j)
{
    std::ofstream ofs(filename);
    ofs << std::setw(4) << j << std::endl;
    return true;
}