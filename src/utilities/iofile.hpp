// 2018.03 Ryotaro Sato

#pragma once
#include <string>
#include <vector>
#include "json.hpp"


template <typename T>
bool dlmread(const std::string &filename, std::vector<std::vector<T> > &Mat);
bool dlmread(const std::string &filename, std::vector<double> &Vec);
bool dlmread(const std::string &filename, double &Val);


template <typename T>
bool dlmwrite(const std::string &filename, std::vector<std::vector<T> > &Mat);
template <typename T>
bool dlmwrite(const std::string &filename, std::vector<T> &Vec);
template <typename T>
bool dlmwrite(const std::string &filename, T &Val);

nlohmann::json jsonread(const std::string &filename);
bool jsonwrite(const std::string &filename, const nlohmann::json &j);
