#ifndef ENERGYSAMPLER_HEADER
#define ENERGYSAMPLER_HEADER

#include "utils.h"
#include <iostream>
#include <vector>
#include <string>
#include <thread>

class energySampler
{
private:
    std::vector<long> times;
    std::vector<long> U;
    std::vector<long> U_IN;
    std::vector<long> I;
    std::vector<long> I_IN;
    std::vector<long> tp;
    FILE *file_u;
    FILE *file_u_in;
    FILE *file_i;
    FILE *file_i_in;
    bool running;
    std::thread runner;

public:
    energySampler();
    ~energySampler();
    void start();
    void end();
    void out(std::string outfilepath = "/data/local/tmp/result.csv");
};

#endif